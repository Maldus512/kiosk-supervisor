import os
import multiprocessing
from pathlib import Path


def PhonyTargets(
    target,
    action,
    depends,
    env=None,
):
    # Creates a Phony target
    if not env:
        env = DefaultEnvironment()
    t = env.Alias(target, depends, action)
    env.AlwaysBuild(t)


# Name of the application
PROGRAM = "supervisor"

# Project paths
MAIN = "main"
COMPONENTS = "components"
CONFIG = f"{MAIN}/config"
LVGL = f"{COMPONENTS}/lvgl"
DRIVERS = f"{COMPONENTS}/lv_drivers"

# Compilation flags
CFLAGS = ["-Wall", "-Wextra", "-g", "-O0", ]
CPPPATH = [COMPONENTS, f"#{MAIN}", f"#{LVGL}", f"#{CONFIG}",
           DRIVERS, f"{COMPONENTS}/log/src"]
CPPDEFINES = ["LV_CONF_INCLUDE_SIMPLE"]


def getIpAddr():
    return ARGUMENTS.get('ip', "")


def main():
    # If not specified, guess how many threads the task can be split into
    num_cpu = multiprocessing.cpu_count()
    SetOption("num_jobs", num_cpu)
    print("Running with -j {}".format(GetOption("num_jobs")))

    debug = ARGUMENTS.get('TARGET_DEBUG', '0')

    env_options = {
        # Include the external environment to access DISPLAY and run the app as a target
        "ENV": os.environ,
        "CPPPATH": CPPPATH,
        "CPPDEFINES": CPPDEFINES,
        "CCFLAGS": CFLAGS,
        "LIBS": ["-lpthread"],
        "CC": "/home/maldus/Mount/Data/Projects/buildrpi/output/host/bin/aarch64-buildroot-linux-uclibc-gcc" if debug == "0" else "gcc"
    }
    env = Environment(**env_options)

    # env['CC'] = ARGUMENTS.get("cc", "gcc")

    if (debug == '0'):
        env['CPPDEFINES'].append(("USE_FBDEV", 1))
        env['CPPDEFINES'].append(("USE_EVDEV", 1))
        env['CPPDEFINES'].append(("USE_SDL", 0))

    if (debug == '1'):
        env['LIBS'].append("-lSDL2")
        env['CPPDEFINES'].append(("USE_SDL", 1))

    env.Tool("compilation_db")

    lv_pman_env = env
    (lv_pman, include) = SConscript(
        f"{COMPONENTS}/lv_page_manager/SConscript", exports=["lv_pman_env"])
    env['CPPPATH'] += [include]

    # Project sources
    sources = [File(filename) for filename in Path(
        f"{MAIN}").rglob("*.c")]  # application files
    sources += [File(filename)
                for filename in Path(f"{LVGL}/src").rglob("*.c")]  # LVGL
    sources += [File(filename)
                for filename in Path(DRIVERS).rglob("*.c")]  # Drivers
    sources += [File(f"{COMPONENTS}/log/src/log.c")]

    prog = env.Program(PROGRAM, sources + lv_pman)

    PhonyTargets("run", f"./{PROGRAM}", prog, env)

    legacy_flags = "-o StrictHostKeyChecking=no -o PubkeyAcceptedAlgorithms=+ssh-rsa"

    PhonyTargets(
        'kill-remote',
        f"ssh {legacy_flags} root@{getIpAddr()} 'killall gdbserver; killall app; killall sh'; true", None)

    PhonyTargets(
        'ssh', 'ssh {} root@{}'.format(legacy_flags, getIpAddr()), None)

    PhonyTargets('scp', 'scp -O {} {} root@{}:/tmp/supervisor'.format(legacy_flags,
                                                                   PROGRAM, getIpAddr()), [prog, "kill-remote"])

    PhonyTargets(
        "run-remote",
        "ssh {} root@{} /tmp/supervisor".format(legacy_flags, getIpAddr()),
        "scp")

    PhonyTargets(
        "debug",
        "ssh {} root@{} gdbserver localhost:1235 /tmp/supervisor".format(legacy_flags, getIpAddr()), "scp")

    compileDB = env.CompilationDatabase('build/compile_commands.json')
    env.Depends(prog, compileDB)


main()
