/*
 * Copyright (c) 2013-2014 RIPE NCC <atlas@ripe.net>
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 * tcputil.h
 */

#include <event2/event_struct.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <event2/bufferevent_ssl.h>

enum tu_err { TU_DNS_ERR, TU_READ_ERR, TU_SOCKET_ERR, TU_CONNECT_ERR,
	TU_OUT_OF_ADDRS, TU_BAD_ADDR, TU_SSL_CTX_INIT_ERR, TU_SSL_OBJ_INIT_ERR,
	TU_SSL_INIT_ERR };

struct tu_env
{
	char dnsip;
	char connecting;
	char host_is_literal;
	struct evutil_addrinfo *dns_res;
	struct evutil_addrinfo *dns_curr;
	struct bufferevent *bev;
	struct timeval interval;
	char *infname;
	char do_tls;
	SSL_CTX *tls_ctx;
	struct event timer;
	struct timespec start_time;	/* name resolution */
	double ttr;
	void (*reporterr)(struct tu_env *env, enum tu_err cause,
		const char *str);
	void (*reportcount)(struct tu_env *env, int count);
	void (*beforeconnect)(struct tu_env *env,
		struct sockaddr *addr, socklen_t addrlen);
	void (*connected)(struct tu_env *env, struct bufferevent *bev);
	void (*readcb)(struct bufferevent *bev, void *env);
	void (*writecb)(struct bufferevent *bev, void *env);
};

extern char *ssl_version;

void tu_connect_to_name(struct tu_env *env, char *host, bool do_tls, char *port,
	struct timeval *timeout,
	struct evutil_addrinfo *hints,
	char *infname,
	void (*timeout_callback)(int unused, const short event, void *env),
	void (*reporterr)(struct tu_env *env, enum tu_err cause,
		const char *err),
	void (*reportcount)(struct tu_env *env, int count),
	void (*beforeconnect)(struct tu_env *env,
		struct sockaddr *addr, socklen_t addrlen),
	void (*connected)(struct tu_env *env, struct bufferevent *bev),
	void (*readcb)(struct bufferevent *bev, void *env),
	void (*writecb)(struct bufferevent *bev, void *env));
void tu_restart_connect(struct tu_env *env);
void tu_fake_ttr(void *ctx, char *host);
void tu_cleanup(struct tu_env *env);
