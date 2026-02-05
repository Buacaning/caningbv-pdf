#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ddi.h"

// CBV: ZeroMQ support for high-performance IPC
#ifdef ENABLE_ZMQ
#include <zmq.h>
#endif

void DDI_set_debug( struct ddi_s *ddi, int debug ) {
	ddi->debug = debug;
}

void DDI_set_prefix( struct ddi_s *ddi, char *prefix ) {
	snprintf(ddi->prefix, sizeof(ddi->prefix), "%s", prefix);
	snprintf(ddi->dispatch_name, sizeof(ddi->dispatch_name), "%s.%s", prefix, DDI_OUT_SUFFIX);
	snprintf(ddi->pickup_name, sizeof(ddi->pickup_name), "%s.%s", prefix, DDI_IN_SUFFIX);
	snprintf(ddi->dispatch_tname, sizeof(ddi->dispatch_tname), "%s.t%s", prefix, DDI_OUT_SUFFIX);
	snprintf(ddi->pickup_tname, sizeof(ddi->pickup_tname), "%s.t%s", prefix, DDI_IN_SUFFIX);
}

void DDI_init( struct ddi_s *ddi ) {
	ddi->debug = 0;
	ddi->resend = 0;
	ddi->prefix[0] = 0;
	ddi->mode = DDI_MODE_NONE;
	ddi->last_dispatch[0] = '\0';
	ddi->last_pickup[0] = '\0';
	// CBV: ZMQ init
	ddi->use_zmq = 0;
	ddi->zmq_context = NULL;
	ddi->zmq_socket = NULL;
	ddi->zmq_endpoint[0] = '\0';
}

// CBV: ZeroMQ implementation
void DDI_set_zmq_endpoint( struct ddi_s *ddi, const char *endpoint ) {
	if (endpoint && endpoint[0]) {
		snprintf(ddi->zmq_endpoint, sizeof(ddi->zmq_endpoint), "%s", endpoint);
		ddi->use_zmq = 1;
		fprintf(stderr, "CBV: DDI ZMQ endpoint set to: %s\n", endpoint);
	} else {
		ddi->use_zmq = 0;
		ddi->zmq_endpoint[0] = '\0';
	}
}

int DDI_zmq_init( struct ddi_s *ddi ) {
#ifdef ENABLE_ZMQ
	if (!ddi->use_zmq || !ddi->zmq_endpoint[0]) {
		return 0;  // Not using ZMQ
	}
	
	// Create ZMQ context
	ddi->zmq_context = zmq_ctx_new();
	if (!ddi->zmq_context) {
		fprintf(stderr, "CBV: ZMQ failed to create context\n");
		ddi->use_zmq = 0;
		return -1;
	}
	
	// Create PAIR socket
	ddi->zmq_socket = zmq_socket(ddi->zmq_context, ZMQ_PAIR);
	if (!ddi->zmq_socket) {
		fprintf(stderr, "CBV: ZMQ failed to create socket\n");
		zmq_ctx_term(ddi->zmq_context);
		ddi->zmq_context = NULL;
		ddi->use_zmq = 0;
		return -1;
	}
	
	// Set socket options
	int linger = 0;
	zmq_setsockopt(ddi->zmq_socket, ZMQ_LINGER, &linger, sizeof(linger));
	
	// Connect to server (BP is always client)
	int rc = zmq_connect(ddi->zmq_socket, ddi->zmq_endpoint);
	if (rc != 0) {
		fprintf(stderr, "CBV: ZMQ failed to connect to %s: %s\n", 
		        ddi->zmq_endpoint, zmq_strerror(zmq_errno()));
		zmq_close(ddi->zmq_socket);
		zmq_ctx_term(ddi->zmq_context);
		ddi->zmq_socket = NULL;
		ddi->zmq_context = NULL;
		ddi->use_zmq = 0;
		return -1;
	}
	
	fprintf(stderr, "CBV: ZMQ connected to %s\n", ddi->zmq_endpoint);
	return 0;
#else
	fprintf(stderr, "CBV: ZMQ not compiled in (ENABLE_ZMQ not defined)\n");
	ddi->use_zmq = 0;
	return -1;
#endif
}

void DDI_zmq_shutdown( struct ddi_s *ddi ) {
#ifdef ENABLE_ZMQ
	if (ddi->zmq_socket) {
		zmq_close(ddi->zmq_socket);
		ddi->zmq_socket = NULL;
	}
	if (ddi->zmq_context) {
		zmq_ctx_term(ddi->zmq_context);
		ddi->zmq_context = NULL;
	}
	ddi->use_zmq = 0;
	fprintf(stderr, "CBV: ZMQ shutdown complete\n");
#endif
}

int DDI_wait( struct ddi_s *ddi, int cycles ) {
	struct stat buffer;   

	while(cycles--) {
		if (stat(ddi->dispatch_name, &buffer)) return 0;
		usleep(10000);
	}
	return -1;
}

void DDI_set_mode( struct ddi_s *ddi, int mode ) {
	ddi->mode = mode;
}


int DDI_dispatch( struct ddi_s *ddi, const char *request ) {
	FILE *fo;
	char *fn, *fnt;

	// CBV: Try ZMQ first if enabled
	if (ddi->use_zmq && ddi->zmq_socket) {
#ifdef ENABLE_ZMQ
		int rc = zmq_send(ddi->zmq_socket, request, strlen(request), ZMQ_DONTWAIT);
		if (rc >= 0) {
			if (ddi->debug) fprintf(stderr, "DDI ZMQ sent: '%s'\n", request);
			if (ddi->resend == 0) snprintf(ddi->last_dispatch, sizeof(ddi->last_dispatch),"%s", request);
			else ddi->resend = 0;
			return 0;
		} else {
			fprintf(stderr, "CBV: ZMQ send failed: %s\n", zmq_strerror(zmq_errno()));
			// Fall through to file-based
		}
#endif
	}

	if (ddi->mode == DDI_MODE_NONE) return 1;

	if (ddi->debug) fprintf(stderr,"DDI Request: '%s'\n", request );

	if (ddi->mode == DDI_MODE_MASTER) {
		fn = ddi->dispatch_name;
		fnt = ddi->dispatch_tname;
	} else if (ddi->mode == DDI_MODE_SLAVE) {
		fn = ddi->pickup_name;
		fnt = ddi->pickup_tname;
	} else {
		// invalid mode
		if (ddi->debug) fprintf(stderr,"Invalid DDI mode (%d)\n", ddi->mode);
		return 2;
	}

	/*
	 * To prevent partial reads of the file, we write to the
	 * temporary file first, then rename it, which is an 
	 * atomic operation
	 */
	fo = fopen(fnt,"w");
	if (fo) {
		fprintf(fo,"%s", request );
		fclose(fo);
		rename(fnt, fn);
	} else {
		if (ddi->debug) fprintf(stderr,"%s:%d: Error trying to open '%s' (%s)\n", __FILE__, __LINE__, fnt, strerror(errno));
		return 1;
	}

	if (ddi->resend == 0) snprintf(ddi->last_dispatch, sizeof(ddi->last_dispatch),"%s", request);
	else ddi->resend = 0;

	return 0;
}

int DDI_resend( struct ddi_s *ddi ){
	ddi->resend = 1;
	return DDI_dispatch( ddi, ddi->last_dispatch );
}

void DDI_clear( struct ddi_s *ddi ) {
	remove(ddi->dispatch_tname);
	remove(ddi->dispatch_name);
	remove(ddi->pickup_tname);
	remove(ddi->pickup_name);
}

#ifndef FL
#define FL __FILE__,__LINE__
#endif

int DDI_pickup( struct ddi_s *ddi, char *buffer, int bsize ) {
	struct stat st;
	FILE *f;
	char *fn;

	buffer[0] = '\0';
	
	// CBV: Try ZMQ first if enabled (non-blocking)
	if (ddi->use_zmq && ddi->zmq_socket) {
#ifdef ENABLE_ZMQ
		int rc = zmq_recv(ddi->zmq_socket, buffer, bsize - 1, ZMQ_DONTWAIT);
		if (rc >= 0) {
			buffer[rc] = '\0';
			if (ddi->debug) fprintf(stderr, "DDI ZMQ received: '%s'\n", buffer);
			snprintf(ddi->last_pickup, sizeof(ddi->last_pickup), "%s", buffer);
			return 0;
		} else if (zmq_errno() != EAGAIN) {
			// Real error (not just no message)
			fprintf(stderr, "CBV: ZMQ recv failed: %s\n", zmq_strerror(zmq_errno()));
		}
		// No message available (EAGAIN), fall through to file-based
#endif
	}
	
	if (ddi->mode == DDI_MODE_NONE) return 1;

	if (ddi->mode == DDI_MODE_SLAVE) {
		fn = ddi->dispatch_name;
	} else {
		fn = ddi->pickup_name;
	}

	stat(fn, &st);

	f = fopen(fn, "r");
	if (f) {
		int rc = 10;
		int rr = 1;
		int bc = 0;
		
		//fgets(buffer, bsize, f);
		bc = fread(buffer, 1, st.st_size, f);
		buffer[bc] = '\0';
		fclose(f);

		while (rc--) {
			rr = unlink(fn); // was remove() before, but changed to unlink for safety
			if ( rr == -1 ) {
				fprintf(stderr,"%s:%d: Unable to remove file (%s) '%s'", FL, strerror(errno), fn);
				usleep(10000);
			} else {
				break;
			}
		}
		snprintf(ddi->last_pickup, sizeof(ddi->last_pickup),"%s",buffer);
	} else {
		if (ddi->debug) fprintf(stderr,"%s:%d: Error trying to open '%s' (%s)\n", __FILE__, __LINE__, fn);
		return 1;
	}

	return 0;
}
