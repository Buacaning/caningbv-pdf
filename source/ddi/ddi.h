#ifdef __cplusplus
extern "C" {
#endif

#ifndef DDI_MODULE
#define DDI_MODULE

// ZeroMQ is now required - file-based DDI removed
#define DDI_MODE_NONE 0
#define DDI_MODE_MASTER 1
#define DDI_MODE_SLAVE 2

#define DDI_LONG_STR 1024

struct ddi_s {
	int resend;
	int mode;
	int debug;
	char last_dispatch[DDI_LONG_STR];
	char last_pickup[DDI_LONG_STR];
	
	// ZeroMQ support (required)
	void *zmq_context;                     // ZMQ context
	void *zmq_socket;                      // ZMQ PAIR socket
	char zmq_endpoint[DDI_LONG_STR];       // ZMQ endpoint (e.g., "ipc:///tmp/cbv_12345_abc.sock")
};

void DDI_init(struct ddi_s *ddi);
void DDI_clear(struct ddi_s *ddi);
void DDI_set_mode(struct ddi_s *ddi, int mode);
void DDI_set_debug(struct ddi_s *ddi, int debug);

// ZMQ functions (required)
void DDI_set_zmq_endpoint(struct ddi_s *ddi, const char *endpoint);
int DDI_zmq_init(struct ddi_s *ddi);
void DDI_zmq_shutdown(struct ddi_s *ddi);

int DDI_dispatch(struct ddi_s *ddi, const char *request);
int DDI_resend(struct ddi_s *ddi);
int DDI_pickup(struct ddi_s *ddi, char *buffer, int bsize);

#endif

#ifdef __cplusplus
}
#endif
