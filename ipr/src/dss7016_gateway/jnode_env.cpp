#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "jnode_env.h"

char *ENV_JROOT    = NULL;
char *ENV_JSYS     = NULL;
char *ENV_JTMP     = NULL;
char *ENV_JRAM     = NULL;
char *ENV_JVAR     = NULL;
char *ENV_JFLAGS   = NULL;

char *ENV_APP_HOME = NULL;
char *ENV_APP_NAME = NULL;

int get_env(const char *name, char **var)
{
    if (*var) { free(*var); *var = NULL; };
    char *s;
    s = getenv(name);
    if (s)  {
        printf("[env] %s = %s\n", name, s);
        *var = strdup(s);
    } else {
        printf("[env] missing %s\n", name);
        return -1;
    }
}

int jnode_env_init()
{
    if (get_env("JROOT",    &ENV_JROOT)    < 0) { return -1; };
    if (get_env("JSYS",     &ENV_JSYS)     < 0) { return -1; };
    if (get_env("JTMP",     &ENV_JTMP)     < 0) { return -1; };
	if (get_env("JRAM",     &ENV_JRAM)     < 0) { return -1; };
    if (get_env("JVAR",     &ENV_JVAR)     < 0) { return -1; };
    if (get_env("JFLAGS",   &ENV_JFLAGS)   < 0) { return -1; };
    if (get_env("APP_HOME", &ENV_APP_HOME) < 0) { return -1; };
    if (get_env("APP_NAME", &ENV_APP_NAME) < 0) { return -1; };
    return 0;
};

int jnode_env_destroy()
{
    if (ENV_JROOT)    { free(ENV_JROOT); }
    if (ENV_JSYS)     { free(ENV_JSYS);  }
    if (ENV_JTMP)     { free(ENV_JTMP);  }
    if (ENV_JRAM)     { free(ENV_JRAM);  }
    if (ENV_JVAR)     { free(ENV_JVAR);  }
    if (ENV_JFLAGS)   { free(ENV_JFLAGS); }

    if (ENV_APP_HOME) { free(ENV_APP_HOME); }
    if (ENV_APP_NAME) { free(ENV_APP_NAME); }

    return 0;
}


int jnode_flag_is_set(const char *name)
{
    char path[128];
	if (ENV_JFLAGS) {
		sprintf(path, "%s/%s", ENV_JFLAGS, name);
	} else {
        printf("[env] missing JFLAGS\n" );
        return false;
	}
    if (access(path, F_OK) < 0) {
        return false;
    } else {
        return true;
    }
};

int jnode_set_flag(const char *name, const int value)
{
    char filename[128];
    if (ENV_JFLAGS) {
		sprintf(filename, "%s/%s", ENV_JFLAGS, name);
	} else {
        printf("[env] missing JFLAGS\n" );
        return -1;
    }

    if (jnode_flag_is_set(name)) {
        if (!value) {
            remove(filename);
        }
    } else {
        if (value) {
            FILE *f = fopen(filename, "w+");
			if (f) {
				fclose(f);
			}            
        }
    }
    return 0;
};
