MAKEFLAGS+=w
PYTHON=uv run
QUARTUS_DIR = C:/intelFPGA_lite/17.0/quartus/bin64
PROJECT = Arcade-TaitoF2
CONFIG = Arcade-TaitoF2
MISTER = root@mister-dev
OUTDIR = output_files
MAME_XML=util/mame.xml
RELEASES_DIR=releases

# Use wsl for submakes on windows
ifeq ($(OS),Windows_NT)
MAKE = wsl make
endif

RBF = $(OUTDIR)/$(CONFIG).rbf

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SRCS_FULL = \
	$(call rwildcard,sys,*.v *.sv *.vhd *.vhdl *.qip *.sdc) \
	$(call rwildcard,rtl,*.v *.sv *.vhd *.vhdl *.qip *.sdc) \
	$(wildcard *.sdc *.v *.sv *.vhd *.vhdl *.qip)

SRCS = $(filter-out %_auto_ss.sv,$(SRCS_FULL))

$(OUTDIR)/Arcade-TaitoF2-Fast.rbf: $(SRCS)
	$(QUARTUS_DIR)/quartus_sh --flow compile $(PROJECT) -c Arcade-TaitoF2-Fast

$(OUTDIR)/Arcade-TaitoF2.rbf: $(SRCS)
	$(QUARTUS_DIR)/quartus_sh --flow compile $(PROJECT) -c Arcade-TaitoF2

deploy.done: $(RBF)
	scp $(RBF) $(MISTER):/media/fat/_Development/cores/TaitoF2.rbf
	echo done > deploy.done

deploy: deploy.done


mister/%: releases/% deploy.done
	scp "$<" $(MISTER):/media/fat/_Development/
	ssh $(MISTER) "echo 'load_core _Development/$(notdir $<)' > /dev/MiSTer_cmd"

mister: mister/F2.mra
mister/finalb: mister/Final\ Blow\ (World).mra
mister/dinorex: mister/Dino\ Rex\ (World).mra
mister/qjinsei: mister/Quiz\ Jinsei\ Gekijoh\ (Japan).mra


rbf: $(OUTDIR)/$(CONFIG).rbf

sim:
	$(MAKE) -j8 -C sim sim

sim/run: sim/deadconx
sim/test: sim/deadconx_test

sim/dinorex:
	$(MAKE) -j8 -C sim run GAME=dinorex

sim/finalb:
	$(MAKE) -j8 -C sim run GAME=finalb

sim/megab:
	$(MAKE) -j8 -C sim run GAME=megab

sim/liquidk:
	$(MAKE) -j8 -C sim run GAME=liquidk

sim/driftout:
	$(MAKE) -j8 -C sim run GAME=driftout

sim/cameltry:
	$(MAKE) -j8 -C sim run GAME=cameltry

sim/pulirula:
	$(MAKE) -j8 -C sim run GAME=pulirula

sim/ninjak:
	$(MAKE) -j8 -C sim run GAME=ninjak

sim/thundfox:
	$(MAKE) -j8 -C sim run GAME=thundfox

sim/deadconx:
	$(MAKE) -j8 -C sim run GAME=deadconx

sim/deadconxj:
	$(MAKE) -j8 -C sim run GAME=deadconxj

sim/metalb:
	$(MAKE) -j8 -C sim run GAME=metalb


sim/qjinsei_test:
	$(MAKE) -j8 -C testroms TARGET=qjinsei_test
	$(MAKE) -j8 -C sim run GAME=qjinsei_test

sim/driftout_test:
	$(MAKE) -j8 -C testroms TARGET=driftout_test
	$(MAKE) -j8 -C sim run GAME=driftout_test

sim/deadconx_test:
	$(MAKE) -j8 -C testroms TARGET=deadconx_test
	$(MAKE) -j8 -C sim run GAME="--script test_script.txt"

sim/finalb_test:
	$(MAKE) -j8 -C testroms TARGET=finalb_test
	$(MAKE) -j8 -C sim run GAME=finalb_test



debug: debug/driftout_test
debug/driftout_test:
	$(MAKE) -j8 -C testroms debug TARGET=driftout_test

debug/deadconx_test:
	$(MAKE) -j8 -C testroms debug TARGET=deadconx_test

debug/finalb_test:
	$(MAKE) -j8 -C testroms debug TARGET=finalb_test



picorom:
	$(MAKE) -j8 -C testroms TARGET=finalb_test picorom


rtl/jt10_auto_ss.sv:
	$(PYTHON) util/state_module.py --generate-csv docs/jt10_mapping.csv jt10 rtl/jt10_auto_ss.sv rtl/jt12/jt49/hdl/*.v rtl/jt12/hdl/adpcm/*.v rtl/jt12/hdl/*.v rtl/jt12/hdl/mixer/*.v

rtl/tv80_auto_ss.sv:
	$(PYTHON) util/state_module.py --generate-csv docs/tv80_mapping.csv tv80s rtl/tv80_auto_ss.sv rtl/tv80/*.v

rtl/fx68k_auto_ss.sv:
	$(PYTHON) util/state_module.py --generate-csv docs/fx68k_mapping.csv fx68k rtl/fx68k_auto_ss.sv rtl/fx68k/hdl/*.v

releases_clean:
	$(PYTHON) util/mame2mra.py --generate --all-machines --output releases_clean --config util/mame2mra.toml util/mame.xml

releases:
	$(PYTHON) util/mame2mra.py --generate --all-machines --output releases --config util/mame2mra.toml util/mame.xml
	patch -E -d releases -l -p1 -r - < releases.patch

releases.patch:
	diff -ruN -x "*.rbf" -x ".DS_Store" releases_clean releases > releases.patch || true

.PHONY: sim sim/run sim/test mister debug picorom rtl/jt10_auto_ss.sv rtl/tv80_auto_ss.sv releases releases_clean releases.patch
