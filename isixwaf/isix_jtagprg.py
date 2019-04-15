#!/usr/bin/env python
# encoding: utf-8

import os

# Configure target
def configure(conf):
    conf.find_program( 'openocd', var='OPENOCD', mandatory=False )

#Options target
def options(ctx):
    pass


#program target
def program( ctx ):
    (ini_cmd,tgt) = _read_openocd_initial_string( ctx )
    tgt = tgt.replace( '\\','/')
    cmd = ctx.env.OPENOCD + [ '-c '+ini_cmd, '-f'+ctx.env.OPENOCD_SCRIPT_FILE,
            '-c program %s verify reset'%tgt,'-c shutdown' ]
    ctx.exec_command( cmd )


#launch openocd debuger
def ocddebug( ctx ):
    print('Openocd started in background check for pid and results')
    (ini_cmd,tgt) = _read_openocd_initial_string( ctx )
    cmd = ctx.env.OPENOCD[0]+' -c "'+ini_cmd+'" -f '+ctx.env.OPENOCD_SCRIPT_FILE
    from multiprocessing import Process
    import platform
    is_windows = any(platform.win32_ver())
    p = Process( target=_openocd_proc, args=( cmd, ),daemon=not is_windows )
    p.start()


# OCD spawned process
def _openocd_proc( args ):
    import subprocess
    DEVNULL = open(os.devnull, 'w')
    ret = subprocess.call( args, shell=True,
            stdout=DEVNULL,
            stderr=subprocess.STDOUT
    )


#Ask for item choice on program
def _ask_for_item_choice(in0):
    from waflib.Errors import WafError
    i = 1
    print("Please select the application:")
    print("[0].\tAbort")
    arr = []
    for key,val in in0.items():
        print("[{:d}].\t{:s}".format(i,key))
        i = i + 1
        arr.append(val)
    try:
        val = int(input())
    except ValueError as err:
        raise WafError('Invalid input', err)
    if val >= i:
        raise WafError('Invalid choice')
    elif val==0:
        raise WafError('Aborted')
    return arr[val-1]

# Read inicmd file
def _read_openocd_initial_string( ctx ):
    from waflib.Errors import WafError
    if not ctx.env.OPENOCD:
        ctx.fatal('Error openocd tool is required for the program target mcu')
    if not ctx.env.OPENOCD_SCRIPT_FILE:
        ctx.fatal('Error openocd script does not exist')
    cfg = ctx.read_default_configuration()
    if not cfg:
        ctx.fatal('Error default configuration does not exist')
    try:
        cfg_tgt = cfg['jtag']['target'];
        if type(cfg_tgt) is str:
            final_tgt = cfg_tgt
        elif type(cfg_tgt) is dict:
            final_tgt = _ask_for_item_choice(cfg_tgt)
        tgt = os.path.join( ctx.out_dir, os.path.normpath(final_tgt))
        family = ctx.isix_get_cpu_family()
        ini_cmd = "set ISIX_INTERFACE %s; " % cfg['jtag']['type']
        ini_cmd += "set ISIX_INTERFACE_TARGET stm32%sx; " % family
        ini_cmd += "set ISIX_INTERFACE_SWD %s; " % cfg['jtag']['swd']
        ini_cmd += "set ISIX_INTERFACE_SPEED_KHZ %s; " % cfg['jtag']['speed']
    except KeyError as err:
        raise WafError('Error invalid jtag config %r'%err, err )
    return ini_cmd,tgt




