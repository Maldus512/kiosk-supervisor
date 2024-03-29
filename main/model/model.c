#include "model.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void model_init(model_t *pmodel) {
    assert(pmodel != NULL);
    model_set_pid(pmodel, -1);
    pmodel->app_version = (char *)malloc(STRSIZE);
    memset(model_get_app_version(pmodel), 0, STRSIZE);

    if (pmodel->app_path == NULL) {
        pmodel->app_path = "";
    }
}
