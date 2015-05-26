#
_XDCBUILDCOUNT = 
ifneq (,$(findstring path,$(_USEXDCENV_)))
override XDCPATH = C:/ti/grace_3_00_03_66/packages;C:/ti/ccsv6/ccs_base;C:/Users/George/workspace_v6_0/qc12/.config
override XDCROOT = C:/ti/xdctools_3_31_00_24_core
override XDCBUILDCFG = ./config.bld
endif
ifneq (,$(findstring args,$(_USEXDCENV_)))
override XDCARGS = 
override XDCTARGETS = 
endif
#
ifeq (0,1)
PKGPATH = C:/ti/grace_3_00_03_66/packages;C:/ti/ccsv6/ccs_base;C:/Users/George/workspace_v6_0/qc12/.config;C:/ti/xdctools_3_31_00_24_core/packages;..
HOSTOS = Windows
endif
