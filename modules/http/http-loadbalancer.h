#ifndef HTTP_LOADBALANCER_H_INCLUDED
#define HTTP_LOADBALANCER_H_INCLUDED 1

#include "syslog-ng.h"

typedef enum
{
  HTTP_TARGET_OPERATIONAL,
  HTTP_TARGET_FAILED
} HTTPLoadBalancerTargetState;

typedef struct _HTTPLoadBalancerTarget HTTPLoadBalancerTarget;
typedef struct _HTTPLoadBalancerClient HTTPLoadBalancerClient;
typedef struct _HTTPLoadBalancer HTTPLoadBalancer;


/* NOTE: this struct represents an actual HTTP target URL.  The existance of
 * this structure is ensured even in multi-threaded environments.  Some of
 * the members of the struct are read-only and are _always_ available
 * without locking.  Others are protected by the LoadBalancer's lock and
 * should be manipulated under the protection of that lock.
 */
struct _HTTPLoadBalancerTarget
{
  /* read-only data, no need to lock */
  gchar *url;
  gint index;
  /* read-write data, locking must be in effect */
  HTTPLoadBalancerTargetState state;
  gint number_of_clients;
  gint max_clients;
  time_t last_failure_time;
};

struct _HTTPLoadBalancerClient
{
  HTTPLoadBalancerTarget *target;
};

void http_lb_client_init(HTTPLoadBalancerClient *, HTTPLoadBalancer *lb);
void http_lb_client_deinit(HTTPLoadBalancerClient *);

struct _HTTPLoadBalancer
{
  GStaticMutex lock;
  HTTPLoadBalancerTarget *targets;
  gint num_targets;
  gint num_clients;
  gint num_failed_targets;
  gint recovery_timeout;
  time_t last_recovery_attempt;
};

HTTPLoadBalancerTarget *http_load_balancer_choose_target(HTTPLoadBalancer *self, HTTPLoadBalancerClient *lbc);
void http_load_balancer_add_target(HTTPLoadBalancer *self, const gchar *url);
void http_load_balancer_drop_all_targets(HTTPLoadBalancer *self);
void http_load_balancer_track_client(HTTPLoadBalancer *self, HTTPLoadBalancerClient *lbc);
void http_load_balancer_set_target_failed(HTTPLoadBalancer *self, HTTPLoadBalancerTarget *target);
void http_load_balancer_set_target_successful(HTTPLoadBalancer *self, HTTPLoadBalancerTarget *target);

void http_load_balancer_set_recovery_timeout(HTTPLoadBalancer *self, gint recovery_timeout);
HTTPLoadBalancer *http_load_balancer_new(void);
void http_load_balancer_free(HTTPLoadBalancer *self);



#endif
