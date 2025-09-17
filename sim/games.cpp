#include "games.h"
#include "sim_core.h"
#include "sim_sdram.h"
#include "sim_ddr.h"
#include "mra_loader.h"

#include "F2.h"
#include "F2___024root.h"

#include "file_search.h"
#include <string.h>
#include <cstdio>

static const char *game_names[N_GAMES] = {
    "finalb",        "dondokod",    "megab",        "thundfox",
    "cameltry",      "qtorimon",    "liquidk",      "quizhq",
    "ssi",           "gunfront",    "growl",        "mjnquest",
    "footchmp",      "koshien",     "yuyugogo",     "ninjak",
    "solfigtr",      "qzquest",     "pulirula",     "metalb",
    "qzchikyu",      "yesnoj",      "deadconx",     "dinorex",
    "qjinsei",       "qcrayon",     "qcrayon2",     "driftout",
    "deadconxj",     "metalba",     "finalb_test",  "qjinsei_test",
    "driftout_test", "deadconx_test",
};

game_t game_find(const char *name)
{
    for (int i = 0; i < N_GAMES; i++)
    {
        if (!strcasecmp(name, game_names[i]))
        {
            return (game_t)i;
        }
    }

    return GAME_INVALID;
}

const char *game_name(game_t game)
{
    if (game == GAME_INVALID)
        return "INVALID";
    return game_names[game];
}

static bool load_audio(const char *name)
{
    std::vector<uint8_t> data;
    if (!g_fs.loadFile(name, data))
    {
        printf("Could not open audio rom %s\n", name);
        return false;
    }

    memcpy(g_sim_core.top->rootp->sim_top__DOT__f2_inst__DOT__sound_rom__DOT__ram.m_storage, data.data(),
           data.size());

    return true;
}

static void load_finalb()
{
    g_fs.addSearchPath("../roms/finalb.zip");

    load_audio("b82_10.ic5");

    g_sim_core.sdram->load_data("b82-09.ic23", CPU_ROM_SDR_BASE + 1, 2);
    g_sim_core.sdram->load_data("b82-17.ic11", CPU_ROM_SDR_BASE + 0, 2);

    g_sim_core.sdram->load_data("b82-07.ic34", SCN0_ROM_SDR_BASE + 1, 2);
    g_sim_core.sdram->load_data("b82-06.ic33", SCN0_ROM_SDR_BASE + 0, 2);

    g_sim_core.sdram->load_data("b82-02.ic1", ADPCMA_ROM_SDR_BASE, 1);
    g_sim_core.sdram->load_data("b82-01.ic2", ADPCMB_ROM_SDR_BASE, 1);

    g_sim_core.ddr_memory->load_data("b82-03.ic9", OBJ_DATA_DDR_BASE + 1, 4);
    g_sim_core.ddr_memory->load_data("b82-04.ic8", OBJ_DATA_DDR_BASE + 0, 4);
    g_sim_core.ddr_memory->load_data("b82-05.ic7", OBJ_DATA_DDR_BASE + 2, 4);
    g_sim_core.ddr_memory->load_data("b82-05.ic7", OBJ_DATA_DDR_BASE + 3, 4);

    g_sim_core.SetGame(GAME_FINALB);
}

static void load_finalb_test()
{
    g_fs.addSearchPath("../testroms/build/finalb_test/finalb/");
    load_finalb();
}

static void load_qjinsei()
{
    g_fs.addSearchPath("../roms/qjinsei.zip");

    load_audio("d48-11");

    g_sim_core.sdram->load_data("d48-09", CPU_ROM_SDR_BASE + 1, 2);
    g_sim_core.sdram->load_data("d48-10", CPU_ROM_SDR_BASE + 0, 2);
    g_sim_core.sdram->load_data("d48-03", CPU_ROM_SDR_BASE + 0x100000, 1);

    g_sim_core.sdram->load_data("d48-04", SCN0_ROM_SDR_BASE, 1);

    g_sim_core.sdram->load_data("d48-05", ADPCMA_ROM_SDR_BASE, 1);

    g_sim_core.ddr_memory->load_data("d48-02", OBJ_DATA_DDR_BASE + 0, 2);
    g_sim_core.ddr_memory->load_data("d48-01", OBJ_DATA_DDR_BASE + 1, 2);

    g_sim_core.SetGame(GAME_QJINSEI);
}

static void load_qjinsei_test()
{
    g_fs.addSearchPath("../testroms/build/qjinsei_test/qjinsei");

    load_qjinsei();
}

static void load_dinorex()
{
    g_fs.addSearchPath("../roms/dinorex.zip");

    load_audio("d39-12.5");

    g_sim_core.sdram->load_data("d39-14.9", CPU_ROM_SDR_BASE + 1, 2);
    g_sim_core.sdram->load_data("d39-16.8", CPU_ROM_SDR_BASE + 0, 2);
    g_sim_core.sdram->load_data("d39-04.6", CPU_ROM_SDR_BASE + 0x100000, 1);
    g_sim_core.sdram->load_data("d39-05.7", CPU_ROM_SDR_BASE + 0x200000, 1);

    g_sim_core.sdram->load_data("d39-06.2", SCN0_ROM_SDR_BASE, 1);

    g_sim_core.sdram->load_data("d39-07.10", ADPCMA_ROM_SDR_BASE, 1);
    g_sim_core.sdram->load_data("d39-08.4", ADPCMB_ROM_SDR_BASE, 1);

    g_sim_core.ddr_memory->load_data("d39-01.29", OBJ_DATA_DDR_BASE, 1);
    g_sim_core.ddr_memory->load_data("d39-02.28", OBJ_DATA_DDR_BASE + 0x200000, 1);
    g_sim_core.ddr_memory->load_data("d39-03.27", OBJ_DATA_DDR_BASE + 0x400000, 1);

    g_sim_core.SetGame(GAME_DINOREX);
}

static void load_liquidk()
{
    g_fs.addSearchPath("../roms/liquidk.zip");
    load_audio("c49-08.ic32");

    g_sim_core.sdram->load_data("c49-09.ic47", CPU_ROM_SDR_BASE + 1, 2);
    g_sim_core.sdram->load_data("c49-11.ic48", CPU_ROM_SDR_BASE + 0, 2);
    g_sim_core.sdram->load_data("c49-10.ic45", CPU_ROM_SDR_BASE + 0x40001, 2);
    g_sim_core.sdram->load_data("c49-12.ic46", CPU_ROM_SDR_BASE + 0x40000, 2);

    g_sim_core.sdram->load_data("c49-03.ic76", SCN0_ROM_SDR_BASE, 1);

    g_sim_core.sdram->load_data("c49-04.ic33", ADPCMA_ROM_SDR_BASE, 1);

    g_sim_core.ddr_memory->load_data("c49-01.ic54", OBJ_DATA_DDR_BASE, 1);
    g_sim_core.ddr_memory->load_data("c49-02.ic53", OBJ_DATA_DDR_BASE + 0x80000, 1);

    g_sim_core.SetGame(GAME_LIQUIDK);
}

static void load_growl()
{
    g_fs.addSearchPath("../roms/growl.zip");

    load_audio("c74-12.ic62");

    g_sim_core.sdram->load_data("c74-10-1.ic59", CPU_ROM_SDR_BASE + 1, 2);
    g_sim_core.sdram->load_data("c74-08-1.ic61", CPU_ROM_SDR_BASE + 0, 2);
    g_sim_core.sdram->load_data("c74-11.ic58", CPU_ROM_SDR_BASE + 0x80001, 2);
    g_sim_core.sdram->load_data("c74-14.ic60", CPU_ROM_SDR_BASE + 0x80000, 2);

    g_sim_core.sdram->load_data("c74-01.ic34", SCN0_ROM_SDR_BASE, 1);

    g_sim_core.sdram->load_data("c74-04.ic28", ADPCMA_ROM_SDR_BASE, 1);
    g_sim_core.sdram->load_data("c74-05.ic29", ADPCMB_ROM_SDR_BASE, 1);

    g_sim_core.ddr_memory->load_data("c74-03.ic12", OBJ_DATA_DDR_BASE, 1);
    g_sim_core.ddr_memory->load_data("c74-02.ic11", OBJ_DATA_DDR_BASE + 0x100000, 1);

    g_sim_core.SetGame(GAME_GROWL);
}

static void load_megab()
{
    g_fs.addSearchPath("../roms/megablst.zip");

    load_audio("c11-12.3");

    g_sim_core.sdram->load_data("c11-07.55", CPU_ROM_SDR_BASE + 1, 2);
    g_sim_core.sdram->load_data("c11-08.39", CPU_ROM_SDR_BASE + 0, 2);
    g_sim_core.sdram->load_data("c11-06.54", CPU_ROM_SDR_BASE + 0x40001, 2);
    g_sim_core.sdram->load_data("c11-11.38", CPU_ROM_SDR_BASE + 0x40000, 2);

    g_sim_core.sdram->load_data("c11-05.58", SCN0_ROM_SDR_BASE, 1);

    g_sim_core.sdram->load_data("c11-01.29", ADPCMA_ROM_SDR_BASE, 1);
    g_sim_core.sdram->load_data("c11-02.30", ADPCMB_ROM_SDR_BASE, 1);

    g_sim_core.ddr_memory->load_data("c11-03.32", OBJ_DATA_DDR_BASE, 2);
    g_sim_core.ddr_memory->load_data("c11-04.31", OBJ_DATA_DDR_BASE + 1, 2);

    g_sim_core.SetGame(GAME_MEGAB);
}

static void load_driftout()
{
    g_fs.addSearchPath("../roms/driftout.zip");

    load_audio("do_50.rom");

    g_sim_core.sdram->load_data("ic46.rom", CPU_ROM_SDR_BASE + 1, 2);
    g_sim_core.sdram->load_data("ic45.rom", CPU_ROM_SDR_BASE + 0, 2);

    g_sim_core.sdram->load_data("do_piv.rom", PIVOT_ROM_SDR_BASE, 1);
    g_sim_core.sdram->load_data("do_snd.rom", ADPCMA_ROM_SDR_BASE, 1);

    g_sim_core.ddr_memory->load_data("do_obj.rom", OBJ_DATA_DDR_BASE, 1);

    g_sim_core.SetGame(GAME_DRIFTOUT);
}

static void load_cameltry()
{
    g_fs.addSearchPath("../roms/cameltry.zip");

    load_audio("c38-08.bin");

    g_sim_core.sdram->load_data("c38-11", CPU_ROM_SDR_BASE + 1, 2);
    g_sim_core.sdram->load_data("c38-14", CPU_ROM_SDR_BASE + 0, 2);

    g_sim_core.sdram->load_data("c38-02.bin", PIVOT_ROM_SDR_BASE, 1);
    g_sim_core.sdram->load_data("c38-03.bin", ADPCMA_ROM_SDR_BASE, 1);

    g_sim_core.ddr_memory->load_data("c38-01.bin", OBJ_DATA_DDR_BASE, 1);

    g_sim_core.SetGame(GAME_CAMELTRY);
}

static void load_driftout_test()
{
    g_fs.addSearchPath("../testroms/build/driftout_test/driftout/");
    load_driftout();

    g_fs.addSearchPath("../roms/growl.zip");
    g_sim_core.sdram->load_data("c74-01.ic34", SCN0_ROM_SDR_BASE, 1);
}

static void load_pulirula()
{
    g_fs.addSearchPath("../roms/pulirula.zip");

    load_audio("c98-14.rom");

    g_sim_core.sdram->load_data("c98-12.rom", CPU_ROM_SDR_BASE + 1, 2);
    g_sim_core.sdram->load_data("c98-16.rom", CPU_ROM_SDR_BASE + 0, 2);
    g_sim_core.sdram->load_data("c98-06.rom", CPU_ROM_SDR_BASE + 0x80001, 2);
    g_sim_core.sdram->load_data("c98-07.rom", CPU_ROM_SDR_BASE + 0x80000, 2);

    g_sim_core.sdram->load_data("c98-04.rom", SCN0_ROM_SDR_BASE, 1);

    g_sim_core.sdram->load_data("c98-05.rom", PIVOT_ROM_SDR_BASE, 1);

    g_sim_core.sdram->load_data("c98-01.rom", ADPCMA_ROM_SDR_BASE, 1);

    g_sim_core.ddr_memory->load_data("c98-02.rom", OBJ_DATA_DDR_BASE, 1);
    g_sim_core.ddr_memory->load_data("c98-03.rom", OBJ_DATA_DDR_BASE + 0x100000, 1);

    g_sim_core.SetGame(GAME_PULIRULA);
}

static void load_ninjak()
{
    g_fs.addSearchPath("../roms/ninjak.zip");

    load_audio("c85-14.ic54");

    g_sim_core.sdram->load_data("c85-10x.ic50", CPU_ROM_SDR_BASE + 1, 2);
    g_sim_core.sdram->load_data("c85-13x.ic49", CPU_ROM_SDR_BASE + 0, 2);
    g_sim_core.sdram->load_data("c85-07.ic48", CPU_ROM_SDR_BASE + 0x40001, 2);
    g_sim_core.sdram->load_data("c85-06.ic47", CPU_ROM_SDR_BASE + 0x40000, 2);

    g_sim_core.sdram->load_data("c85-03.ic65", SCN0_ROM_SDR_BASE, 1);

    g_sim_core.sdram->load_data("c85-04.ic31", ADPCMA_ROM_SDR_BASE, 1);
    g_sim_core.sdram->load_data("c85-05.ic33", ADPCMB_ROM_SDR_BASE, 1);

    g_sim_core.ddr_memory->load_data("c85-01.ic19", OBJ_DATA_DDR_BASE, 1);
    g_sim_core.ddr_memory->load_data("c85-02.ic17", OBJ_DATA_DDR_BASE + 0x100000, 1);

    g_sim_core.SetGame(GAME_NINJAK);
}

static void load_thundfox()
{
    g_fs.addSearchPath("../roms/thundfox.zip");

    load_audio("c28-14.3");

    g_sim_core.sdram->load_data("c28-13-1.51", CPU_ROM_SDR_BASE + 1, 2);
    g_sim_core.sdram->load_data("c28-16-1.40", CPU_ROM_SDR_BASE + 0, 2);
    g_sim_core.sdram->load_data("c28-08.50", CPU_ROM_SDR_BASE + 0x40001, 2);
    g_sim_core.sdram->load_data("c28-07.39", CPU_ROM_SDR_BASE + 0x40000, 2);

    g_sim_core.sdram->load_data("c28-02.61", SCN0_ROM_SDR_BASE, 1);
    g_sim_core.sdram->load_data("c28-01.63", SCN1_ROM_SDR_BASE, 1);

    g_sim_core.sdram->load_data("c28-06.41", ADPCMA_ROM_SDR_BASE, 1);
    g_sim_core.sdram->load_data("c28-05.42", ADPCMB_ROM_SDR_BASE, 1);

    g_sim_core.ddr_memory->load_data("c28-03.29", OBJ_DATA_DDR_BASE, 2);
    g_sim_core.ddr_memory->load_data("c28-04.28", OBJ_DATA_DDR_BASE + 0x1, 2);

    g_sim_core.SetGame(GAME_THUNDFOX);
}

static void load_deadconx()
{
    g_fs.addSearchPath("../roms/deadconx.zip");

    load_audio("d28-10.6");

    g_sim_core.sdram->load_data("d28-06.3", CPU_ROM_SDR_BASE + 1, 2);
    g_sim_core.sdram->load_data("d28-12.5", CPU_ROM_SDR_BASE + 0, 2);
    g_sim_core.sdram->load_data("d28-09.2", CPU_ROM_SDR_BASE + 0x80001, 2);
    g_sim_core.sdram->load_data("d28-08.4", CPU_ROM_SDR_BASE + 0x80000, 2);

    g_sim_core.sdram->load_data16be("d28-04.16", SCN1_ROM_SDR_BASE + 0x2, 4);
    g_sim_core.sdram->load_data16be("d28-05.17", SCN1_ROM_SDR_BASE, 4);

    g_sim_core.sdram->load_data("d28-03.10", ADPCMA_ROM_SDR_BASE, 1);

    g_sim_core.ddr_memory->load_data("d28-01.8", OBJ_DATA_DDR_BASE, 1);
    g_sim_core.ddr_memory->load_data("d28-02.9", OBJ_DATA_DDR_BASE + 0x100000, 1);

    g_sim_core.SetGame(GAME_DEADCONX);
}

static void load_deadconxj()
{
    load_deadconx();

    g_fs.addSearchPath("../roms/deadconxj.zip");

    g_sim_core.sdram->load_data("d28-07.5", CPU_ROM_SDR_BASE + 0, 2);

    g_sim_core.SetGame(GAME_DEADCONXJ);
}

static void load_deadconxj_test()
{
    g_fs.addSearchPath("../testroms/build/deadconx_test/deadconxj/");
    load_deadconxj();
}

static void load_metalb()
{
    g_fs.addSearchPath("../roms/metalb.zip");

    load_audio("d12-13.5");

    g_sim_core.sdram->load_data("d16-16.8", CPU_ROM_SDR_BASE + 1, 2);
    g_sim_core.sdram->load_data("d16-18.7", CPU_ROM_SDR_BASE + 0, 2);
    g_sim_core.sdram->load_data("d12-07.9", CPU_ROM_SDR_BASE + 0x80001, 2);
    g_sim_core.sdram->load_data("d12-06.6", CPU_ROM_SDR_BASE + 0x80000, 2);

    g_sim_core.sdram->load_data16be("d12-03.14", SCN1_ROM_SDR_BASE + 0x2, 4);
    g_sim_core.sdram->load_data16be("d12-04.13", SCN1_ROM_SDR_BASE, 4);

    g_sim_core.sdram->load_data("d12-02.10", ADPCMA_ROM_SDR_BASE, 1);
    g_sim_core.sdram->load_data("d12-05.16", ADPCMB_ROM_SDR_BASE, 1);

    g_sim_core.ddr_memory->load_data("d12-01.20", OBJ_DATA_DDR_BASE, 1);

    g_sim_core.SetGame(GAME_METALB);
}

bool game_init(game_t game)
{
    g_fs.clearSearchPaths();
    g_fs.addSearchPath(".");

    switch (game)
    {
    case GAME_FINALB:
        load_finalb();
        break;
    case GAME_QJINSEI:
        load_qjinsei();
        break;
    case GAME_LIQUIDK:
        load_liquidk();
        break;
    case GAME_DINOREX:
        load_dinorex();
        break;
    case GAME_FINALB_TEST:
        load_finalb_test();
        break;
    case GAME_QJINSEI_TEST:
        load_qjinsei_test();
        break;
    case GAME_GROWL:
        load_growl();
        break;
    case GAME_MEGAB:
        load_megab();
        break;
    case GAME_DRIFTOUT:
        load_driftout();
        break;
    case GAME_DRIFTOUT_TEST:
        load_driftout_test();
        break;
    case GAME_CAMELTRY:
        load_cameltry();
        break;
    case GAME_PULIRULA:
        load_pulirula();
        break;
    case GAME_NINJAK:
        load_ninjak();
        break;
    case GAME_THUNDFOX:
        load_thundfox();
        break;
    case GAME_DEADCONX:
        load_deadconx();
        break;
    case GAME_DEADCONXJ:
        load_deadconxj();
        break;
    case GAME_DEADCONX_TEST:
        load_deadconxj_test();
        break;
    case GAME_METALB:
        load_metalb();
        break;
    default:
        return false;
    }

    return true;
}

bool game_init_mra(const char *mra_path)
{
    g_fs.clearSearchPaths();
    
    // Add common ROM search paths
    std::vector<std::string> searchPaths = {
        ".",
        "../roms/"
    };
    
    // Add ROM search paths
    for (const auto& path : searchPaths) {
        g_fs.addSearchPath(path);
    }
    
    // Add the directory containing the MRA file as a search path
    std::string mraPathStr(mra_path);
    size_t lastSlash = mraPathStr.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        g_fs.addSearchPath(mraPathStr.substr(0, lastSlash));
    }
    
    // Load the MRA file
    MRALoader loader;
    std::vector<uint8_t> romData;
    uint32_t address = 0;
    
    if (!loader.load(mra_path, romData, address)) {
        printf("Failed to load MRA file '%s': %s\n", mra_path, loader.getLastError().c_str());
        return false;
    }
    
    printf("Loaded MRA: %s\n", mra_path);
    printf("ROM data size: %zu bytes\n", romData.size());
   
    if (address == 0) {
        if (!g_sim_core.SendIOCTLData(0, romData)) {
            printf("Failed to send ROM data via ioctl\n");
            return false;
        }
    } else {
        if (!g_sim_core.SendIOCTLDataDDR(0, address, romData)) {
            printf("Failed to send ROM data via DDR\n");
            return false;
        }
    }
    
    printf("Successfully loaded MRA: %s\n", mra_path);
    return true;
}
