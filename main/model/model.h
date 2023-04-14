#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include "utils/socketq.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

#define STRSIZE 1024

#define GETTER(name, field)                                                    \
  static inline                                                                \
      __attribute__((always_inline, const)) typeof(((model_t *)0)->field)      \
          model_get_##name(model_t *pmodel) {                                  \
    assert(pmodel != NULL);                                                    \
    return pmodel->field;                                                      \
  }

#define SETTER(name, field)                                                    \
  static inline __attribute__((always_inline)) uint8_t model_set_##name(       \
      model_t *pmodel, typeof(((model_t *)0)->field) value) {                  \
    assert(pmodel != NULL);                                                    \
    if (pmodel->field != value) {                                              \
      pmodel->field = value;                                                   \
      return 1;                                                                \
    } else {                                                                   \
      return 0;                                                                \
    }                                                                          \
  }

#define TOGGLER(name, field)                                                   \
  static inline __attribute__((always_inline)) void model_toggle_##name(       \
      model_t *pmodel) {                                                       \
    assert(pmodel != NULL);                                                    \
    pmodel->field = !pmodel->field;                                            \
  }

#define GETTERNSETTER(name, field)                                             \
  GETTER(name, field)                                                          \
  SETTER(name, field)

typedef struct {
  char *sup_version;
  char *app_version;
  char *app_path;
  char *update_path;
  char **log_paths;
  int n_log_paths;
  char *mount_path;
  socketq_t *fsocketq; // used for fork
  socketq_t *tsocketq; // used for thread
  socketq_t *msocketq; // used for main

  uint period;
  uint term_per_period;

  uint black_screen_timer;

  pid_t pid;

  bool settings_open;
  bool msgbox_update_open;
} model_t;

void model_init(model_t *pmodel);

GETTER(sup_version, sup_version)
GETTERNSETTER(app_version, app_version)
GETTERNSETTER(app_path, app_path)
GETTER(update_path, update_path)
GETTER(log_paths, log_paths)
GETTER(n_log_paths, n_log_paths)
GETTERNSETTER(mount_path, mount_path)
GETTER(fsocketq, fsocketq)
GETTER(tsocketq, tsocketq)
GETTER(msocketq, msocketq)
GETTER(period, period)
GETTER(term_per_period, term_per_period)
GETTER(black_screen_timer, black_screen_timer)
GETTERNSETTER(pid, pid)
GETTERNSETTER(settings_open, settings_open)
GETTERNSETTER(msgbox_update_open, msgbox_update_open)

#endif
