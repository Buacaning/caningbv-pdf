#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ddi.h"
#include <zmq.h>

void DDI_set_debug(struct ddi_s *ddi, int debug) {
	ddi->debug = debug;
}

void DDI_init(struct ddi_s *ddi) {
	ddi->debug = 0;
	ddi->resend = 0;
	ddi->mode = DDI_MODE_NONE;
	ddi->last_dispatch[0] = '\0';
	ddi->last_pickup[0] = '\0';
	// ZMQ init
	ddi->zmq_context = NULL;
	ddi->zmq_socket = NULL;
	ddi->zmq_endpoint[0] = '\0';
}

// ZeroMQ implementation
void DDI_set_zmq_endpoint(struct ddi_s *ddi, const char *endpoint) {
	if (endpoint && endpoint[0]) {
		snprintf(ddi->zmq_endpoint, sizeof(ddi->zmq_endpoint), "%s", endpoint);
		fprintf(stderr, "CBV: DDI ZMQ endpoint set to: %s\n", endpoint);
	} else {
		ddi->zmq_endpoint[0] = '\0';
	}
}

int DDI_zmq_init(struct ddi_s *ddi) {
	if (!ddi->zmq_endpoint[0]) {
		fprintf(stderr, "CBV: ZMQ endpoint not set\n");
		return -1;
	}
	
	// Create ZMQ context
	ddi->zmq_context = zmq_ctx_new();
	if (!ddi->zmq_context) {
		fprintf(stderr, "CBV: ZMQ failed to create context\n");
		return -1;
	}
	
	// Create PAIR socket
	ddi->zmq_socket = zmq_socket(ddi->zmq_context, ZMQ_PAIR);
	if (!ddi->zmq_socket) {
		fprintf(stderr, "CBV: ZMQ failed to create socket\n");
		zmq_ctx_term(ddi->zmq_context);
		ddi->zmq_context = NULL;
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
		return -1;
	}
	
	fprintf(stderr, "CBV: ZMQ connected to %s\n", ddi->zmq_endpoint);
	return 0;
}

void DDI_zmq_shutdown(struct ddi_s *ddi) {
	if (ddi->zmq_socket) {
		zmq_close(ddi->zmq_socket);
		ddi->zmq_socket = NULL;
	}
	if (ddi->zmq_context) {
		zmq_ctx_term(ddi->zmq_context);
		ddi->zmq_context = NULL;
	}
	fprintf(stderr, "CBV: ZMQ shutdown complete\n");
}

void DDI_set_mode(struct ddi_s *ddi, int mode) {
	ddi->mode = mode;
}

int DDI_dispatch(struct ddi_s *ddi, const char *request) {
	if (!ddi->zmq_socket) {
		fprintf(stderr, "CBV: ZMQ not initialized\n");
		return 1;
	}
	
	if (ddi->debug) fprintf(stderr, "DDI Request: '%s'\n", request);
	
	int rc = zmq_send(ddi->zmq_socket, request, strlen(request), ZMQ_DONTWAIT);
	if (rc < 0) {
		fprintf(stderr, "CBV: ZMQ send failed: %s\n", zmq_strerror(zmq_errno()));
		return 1;
	}
	
	if (ddi->debug) fprintf(stderr, "DDI ZMQ sent: '%s'\n", request);
	
	if (ddi->resend == 0) {
		snprintf(ddi->last_dispatch, sizeof(ddi->last_dispatch), "%s", request);
	} else {
		ddi->resend = 0;
	}
	
	return 0;
}

int DDI_resend(struct ddi_s *ddi) {
	ddi->resend = 1;
	return DDI_dispatch(ddi, ddi->last_dispatch);
}

void DDI_clear(struct ddi_s *ddi) {
	// Nothing to clear with ZMQ
}

int DDI_pickup(struct ddi_s *ddi, char *buffer, int bsize) {
	buffer[0] = '\0';
	
	if (!ddi->zmq_socket) {
		return 1;
	}
	
	int rc = zmq_recv(ddi->zmq_socket, buffer, bsize - 1, ZMQ_DONTWAIT);
	if (rc < 0) {
		if (zmq_errno() == EAGAIN) {
			// No message available
			return 1;
		}
		// Real error
		fprintf(stderr, "CBV: ZMQ recv failed: %s\n", zmq_strerror(zmq_errno()));
		return 1;
	}
	
	buffer[rc] = '\0';
	if (ddi->debug) fprintf(stderr, "DDI ZMQ received: '%s'\n", buffer);
	snprintf(ddi->last_pickup, sizeof(ddi->last_pickup), "%s", buffer);
	
	return 0;
}
