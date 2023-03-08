#include "controller.h"
#include "controller/message.h"
#include "log.h"
#include "lv_page_manager.h"
#include "lv_page_manager_conf.h"
#include "model/model.h"
#include "utils/socketq.h"
#include "view/view.h"
#include <fcntl.h>
#include <libgen.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

unsigned long start;
uint term;

bool copy_file(const char *source_path, const char *dest_path) {

  int read_fd;
  int write_fd;
  struct stat stat_buf;
  off_t offset = 0;

  /* Open the input file. */
  read_fd = open(source_path, O_RDONLY);
  if (read_fd == -1) {
    log_info("non riesco a leggere dal file di log %s", source_path);
    return false;
  }
  /* Stat the input file to obtain its size. */
  fstat(read_fd, &stat_buf);
  /* Open the output file for writing, with the same permissions as the
    source file. */
  write_fd = open(dest_path, O_WRONLY | O_CREAT, stat_buf.st_mode);
  if (write_fd == -1) {
    log_info("non riesco a scrivere nel file di log %s", source_path);
    return false;
  }
  /* Blast the bytes from one file to the other. */
  sendfile(write_fd, read_fd, &offset, stat_buf.st_size);
  /* Close up. */

  close(read_fd);
  close(write_fd);

  return true;
}

bool file_exe(const char *path) { return access(path, X_OK) == 0; }

void get_app_version(const char *path, char *buffer) {

  if (!file_exe(path)) {
    // just "" crashes
    strcpy(buffer, " ");
    log_error("errore versione: file non eseguibile");
    return;
  }

  pid_t pid;

  int pipefd[2];
  pipe(pipefd);

  // child
  if ((pid = fork()) == -1) {
    log_error("errore creazione fork");
  } else if (pid == 0) {

    // kill me if parent dies
    prctl(PR_SET_PDEATHSIG, SIGTERM);

    close(pipefd[0]); // close reading end in the child

    dup2(pipefd[1], STDOUT_FILENO); // send stdout to the pipe
    // dup2(pipefd[1], STDERR_FILENO); // send stderr to the pipe

    close(pipefd[1]); // this descriptor is no longer needed

    const char *argv[3] = {path, "-v", NULL};
    execve(argv[0], (char **)argv, NULL);
  } else {

    close(pipefd[1]); // close the write end of the pipe in the parent

    // while (read(pipefd[0], buffer, STRSIZE) != 0) {
    // }
    // read only the first STRSIZE bytes
    read(pipefd[0], buffer, STRSIZE);

    int status;
    waitpid(pid, &status, WUNTRACED);
  }
}

void controller_io(model_t *pmodel) {
  uint8_t msg = 0;
  for (;;) {
    socketq_receive((socketq_t *)model_get_tsocketq(pmodel), &msg);
    switch (msg) {
    case LV_PMAN_CONTROLLER_MSG_TAG_EXPORT_LOGS: {
      bool r = true;
      for (int i = 0; i < model_get_n_log_paths(pmodel); i++) {
        // checks if exists
        char *name = basename(model_get_log_paths(pmodel)[i]);
        char *fullpath =
            malloc(strlen(model_get_log_exp_path(pmodel)) + strlen(name) + 2);
        if (fullpath == NULL) { /* deal with error and exit */
        }
        sprintf(fullpath, "%s/%s", model_get_log_exp_path(pmodel), name);
        log_info("esporto %s", fullpath);
        r &= copy_file(model_get_log_paths(pmodel)[i], fullpath);

        free(fullpath);

        if (!r)
          break;
      }

      uint8_t msg = LV_PMAN_CONTROLLER_MSG_TAG_EXPORT_LOGS_SUCC;
      if (!r)
        msg = LV_PMAN_CONTROLLER_MSG_TAG_EXPORT_LOGS_ERR;
      socketq_send((socketq_t *)model_get_msocketq(pmodel), &msg);

      break;
    }
    case LV_PMAN_CONTROLLER_MSG_TAG_UPDATE_COPY: {
      log_info("copio");
      if (copy_file(model_get_update_path(pmodel),
                    model_get_app_path(pmodel))) {
        log_info("copiato con successo");

        if (pmodel->pid != -1) {
          log_info("applicazione in esecuzione");
          log_info("pid %d\n", model_get_pid(pmodel));
          kill(pmodel->pid, SIGKILL);
        }

        model_set_pid(pmodel, -1);

        // kill previous running app
        get_app_version(model_get_app_path(pmodel),
                        model_get_app_version(pmodel));

        view_app_update_success_event();

        controller_start_app(pmodel);
      }
      break;
    }
    }
  }
  pthread_exit(NULL);
}

void controller_fork(model_t *pmodel) {
  uint8_t msg = 99;
  for (;;) {
    socketq_receive((socketq_t *)(pmodel->fsocketq), &msg);
    pid_t pid;

    int res = -1;
    // checks if exists
    if ((res = access(model_get_app_path(pmodel), X_OK)) != 0) {
      log_info("app non trovata in %s (%d)", model_get_app_path(pmodel), res);
      uint8_t msg = LV_PMAN_CONTROLLER_MSG_TAG_APP_EXIT_ERROR;
      socketq_send((socketq_t *)model_get_msocketq(pmodel), &msg);
      continue;
    }

    // child
    if ((pid = fork()) == -1) {
      log_error("errore creazione fork");
    } else if (pid == 0) {
      // kill me if parent dies
      prctl(PR_SET_PDEATHSIG, SIGTERM);

      log_info("avvio in app %d", msg);
      const char *argv[2] = {model_get_app_path(pmodel), NULL};
      execve(argv[0], (char **)argv, NULL);
    } else {
      model_set_pid(pmodel, pid);
      int status;
      waitpid(pid, &status, WUNTRACED);
      model_set_pid(pmodel, -1);
      uint8_t msg;

      if (WIFEXITED(status)) {
        int es = WEXITSTATUS(status);
        // view_app_started_successfully_event();
        if (es == 0) {
          if (get_millis() - start < model_get_period(pmodel) &&
              ++term >= model_get_term_per_period(pmodel)) {
            msg = LV_PMAN_CONTROLLER_MSG_TAG_APP_EXIT_MANY_TIMES;
            log_info("troppe volte");
            term = 0;
          } else {
            start = get_millis();
            msg = LV_PMAN_CONTROLLER_MSG_TAG_APP_START;
          }
        } else {
          log_info("error");
          msg = LV_PMAN_CONTROLLER_MSG_TAG_APP_EXIT_ERROR;
        }
      }
      socketq_send((socketq_t *)model_get_msocketq(pmodel), &msg);
    }
  }
  pthread_exit(NULL);
}

void controller_export_logs(model_t *pmodel) {
  log_info("esporto logs");
  uint8_t e = LV_PMAN_CONTROLLER_MSG_TAG_EXPORT_LOGS;
  socketq_send((socketq_t *)model_get_tsocketq(pmodel), &e);
}

// this function could block the execution for a bit since
// -v should return only the version, then exit the program
void controller_get_app_version(model_t *pmodel) {
  log_info("versione applicazione");

  char buffer[1024] = {0};

  get_app_version(model_get_app_path(pmodel), buffer);

  model_set_app_version(pmodel, buffer);
  printf("vers %s\n", model_get_app_version(pmodel));
}

bool need_update_app(const char *path, const char *curr_ver) {

  char buffer[1024] = {0};

  get_app_version(path, buffer);

  return buffer[0] != '\0' && strcmp(curr_ver, buffer) != 0;
}

void controller_update_app(model_t *pmodel) {
  log_info("aggiorno applicazione (%s)", model_get_app_version(pmodel));

  if (!file_exe(model_get_update_path(pmodel))) {
    log_error("errore versione: file non eseguibile");
    view_app_update_error_event();
    return;
  }
  if (!need_update_app(model_get_update_path(pmodel),
                       model_get_app_version(pmodel))) {
    log_info("applicazione gia' aggiornata");
    view_app_update_already_event();
    return;
  }

  // TODO: deve per forza passare dalla view o puo' direttamente parlare con
  // il controller ???
  // uint8_t msg = LV_PMAN_CONTROLLER_MSG_TAG_APP_UPDATE_FOUND;
  // socketq_send((socketq_t *)model_get_msocketq(pmodel), &msg);
  view_app_update_found_event();

  uint8_t msg = LV_PMAN_CONTROLLER_MSG_TAG_UPDATE_COPY;
  socketq_send((socketq_t *)model_get_tsocketq(pmodel), &msg);
}

void controller_init(model_t *pmodel) {

  pthread_t io_id;
  pthread_create(&io_id, NULL, (void *(*)(void *))controller_io, pmodel);
  pthread_t fork_id;
  pthread_create(&fork_id, NULL, (void *(*)(void *))controller_fork, pmodel);

  start = get_millis();
  term = 0;

  get_app_version(model_get_app_path(pmodel), model_get_app_version(pmodel));
  log_info("ver (%s)", model_get_app_version(pmodel));

  view_change_page(pmodel, page_black);
  controller_start_app(pmodel);
}

void controller_manage_message(void *args, lv_pman_controller_msg_t msg) {
  model_t *pmodel = args;

  switch (msg.tag) {
  case LV_PMAN_CONTROLLER_MSG_TAG_NONE:
    break;
  case LV_PMAN_CONTROLLER_MSG_TAG_APP_START:
    controller_start_app(pmodel);
    break;
  case LV_PMAN_CONTROLLER_MSG_TAG_EXPORT_LOGS:
    controller_export_logs(pmodel);
    break;
  case LV_PMAN_CONTROLLER_MSG_TAG_APP_VERSION:
    controller_get_app_version(pmodel);
    break;
  case LV_PMAN_CONTROLLER_MSG_TAG_APP_UPDATE:
    controller_update_app(pmodel);
    break;
  case LV_PMAN_CONTROLLER_MSG_TAG_OPEN_SETTINGS:
    log_info("open settings");
    controller_stop_app(pmodel);
    controller_open_settings(pmodel);
    break;
  }
}

void controller_open_settings(model_t *pmodel) {
  view_change_page(pmodel, page_settings);
}

void controller_manage(model_t *pmodel) {

  // if (model_get_settings_open(pmodel) &&
  //     !model_get_msgbox_update_open(pmodel) &&
  //     need_update_app(model_get_update_path(pmodel),
  //                     model_get_app_version(pmodel))) {
  //   log_info("applicazione da aggiornare");
  //   view_app_update_found_event();
  //   model_set_msgbox_update_open(pmodel, true);
  //   // controller_update_app(pmodel);
  // }

  uint8_t msg = 0;
  socketq_receive_nonblock((socketq_t *)model_get_msocketq(pmodel), &msg, 10);
  if (msg != 0) {
    switch (msg) {
    case LV_PMAN_CONTROLLER_MSG_TAG_APP_START: {
      controller_start_app(pmodel);
      break;
    }
    case LV_PMAN_CONTROLLER_MSG_TAG_APP_UPDATE_SUCC: {
      view_app_update_success_event();
      break;
    }
    case LV_PMAN_CONTROLLER_MSG_TAG_APP_EXIT_MANY_TIMES: {
      view_change_page(pmodel, page_settings);
      view_app_exit_many_times_event();
      break;
    }
    case LV_PMAN_CONTROLLER_MSG_TAG_APP_EXIT_ERROR: {
      view_change_page(pmodel, page_settings);
      view_app_exit_error_event();
      break;
    }
    case LV_PMAN_CONTROLLER_MSG_TAG_EXPORT_LOGS_SUCC: {
      view_export_logs_success_event();
      break;
    }
    case LV_PMAN_CONTROLLER_MSG_TAG_EXPORT_LOGS_ERR: {
      view_export_logs_error_event();
      break;
    }
    }
  }
}

void controller_start_app(model_t *pmodel) {
  if (model_get_pid(pmodel) == -1) {
    log_info("avvio applicazione");
    uint8_t e = 3;
    socketq_send((socketq_t *)model_get_fsocketq(pmodel), &e);
  } else {
    log_info("applicazione gia' avviata");
    view_app_start_already_event();
  }
}

void controller_stop_app(model_t *pmodel) {
  if (pmodel->pid != -1) {
    log_info("fermo pid %d\n", model_get_pid(pmodel));
    kill(pmodel->pid, SIGKILL);
    model_set_pid(pmodel, -1);
  }
}
