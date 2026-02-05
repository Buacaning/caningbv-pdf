#ifdef __cplusplus
extern "C" {
#endif

#ifndef DDI_MODULE
#define DDI_MODULE

#define DDI_MODE_NONE 0
#define DDI_MODE_MASTER 1
#define DDI_MODE_SLAVE 2

#define DDI_OUT_SUFFIX "ddo"
#define DDI_IN_SUFFIX "ddi"

#define DDI_LONG_STR 1024

struct ddi_s {
	int resend;
	int mode;
	int debug;
	char last_dispatch[DDI_LONG_STR];
	char last_pickup[DDI_LONG_STR];

	char prefix[DDI_LONG_STR];
	char dispatch_name[DDI_LONG_STR];
	char pickup_name[DDI_LONG_STR];
	char dispatch_tname[DDI_LONG_STR];
	char pickup_tname[DDI_LONG_STR];
	
	// ZMQ support (CBV: ZeroMQ integration)
	int use_zmq;                           // 1 = use ZMQ, 0 = file-based
	void *zmq_context;                     // ZMQ context
	void *zmq_socket;                      // ZMQ PAIR socket
	char zmq_endpoint[DDI_LONG_STR];       // ZMQ endpoint (e.g., "tcp://127.0.0.1:12345")
};

void DDI_init( struct ddi_s *ddi );
void DDI_clear( struct ddi_s *ddi );
int DDI_wait( struct ddi_s *ddi, int cycles );
void DDI_set_mode( struct ddi_s *ddi, int mode );
void DDI_set_debug( struct ddi_s *ddi, int debug );
void DDI_set_prefix( struct ddi_s *ddi, char *prefix );

// ZMQ functions (CBV: ZeroMQ integration)
void DDI_set_zmq_endpoint( struct ddi_s *ddi, const char *endpoint );
int DDI_zmq_init( struct ddi_s *ddi );
void DDI_zmq_shutdown( struct ddi_s *ddi );

int DDI_dispatch( struct ddi_s *ddi, const char *request ); 
int DDI_resend( struct ddi_s *ddi );
int DDI_pickup( struct ddi_s *ddi, char *buffer, int bsize );

#endif

#ifdef __cplusplus
}
#endif
