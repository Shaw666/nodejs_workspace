#ifndef JNODE_ENV_H
#define JNODE_ENV_H

extern char *ENV_JROOT;
extern char *ENV_JSYS;
extern char *ENV_JTMP;
extern char *ENV_JRAM;
extern char *ENV_JVAR;
extern char *ENV_JFLAGS;
extern char *ENV_JAPPS;

extern char *ENV_APP_HOME;
extern char *ENV_APP_NAME;

extern int jnode_env_init();
extern int jnode_flag_is_set(const char *name);
extern int jnode_set_flag(const char *name, const int value);
extern int jnode_env_destroy();

#endif
