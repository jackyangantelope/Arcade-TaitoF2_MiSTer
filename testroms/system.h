#if !defined (SYSTEM_H)
#define SYSTEM_H 1

#include "tc0100scn.h"
#include "tc0220ioc.h"
#include "te7750.h"
#include "tc0200obj.h"
#include "tc0360pri.h"
#include "tc0430grw.h"
#include "tc0480scp.h"

#if GAME_FINALB
#define HAS_TC0110PCR 1
#define HAS_TC0220IOC 1
#define HAS_TC0100SCN 1

static volatile uint16_t *TC0110PCR_ADDR = (volatile uint16_t *)0x200000;
static volatile uint16_t *TC0110PCR_DATA = (volatile uint16_t *)0x200002;
static volatile uint16_t *TC0110PCR_WHAT = (volatile uint16_t *)0x200004;

static TC0220IOC_Control *TC0220IOC = (TC0220IOC_Control *)0x300000;

static TC0100SCN_Layout *TC0100SCN = (TC0100SCN_Layout *)0x800000;
static TC0100SCN_Control *TC0100SCN_Ctrl = (TC0100SCN_Control *)0x820000;

static TC0200OBJ_Inst *TC0200OBJ = (TC0200OBJ_Inst *)0x900000;

static volatile uint8_t *SYT_ADDR = (volatile uint8_t *)0x320000;
static volatile uint8_t *SYT_DATA = (volatile uint8_t *)0x320002;

#elif GAME_SSI

#define HAS_TC0260DAR 1
#define HAS_TC0220IOC 1
#define HAS_TC0100SCN 1

static volatile uint16_t *TC0260DAR = (volatile uint16_t *)0x300000;

static TC0100SCN_Layout *TC0100SCN = (TC0100SCN_Layout *)0x600000;
static TC0100SCN_Control *TC0100SCN_Ctrl = (TC0100SCN_Control *)0x620000;

static TC0220IOC_Control *TC0220IOC = (TC0220IOC_Control *)0x100000;

static TC0200OBJ_Inst *TC0200OBJ = (TC0200OBJ_Inst *)0x800000;

static volatile uint8_t *SYT_ADDR = (volatile uint8_t *)0x320000;
static volatile uint8_t *SYT_DATA = (volatile uint8_t *)0x320002;

#elif GAME_QJINSEI

#define HAS_TC0260DAR 1
#define HAS_TC0360PRI 1
#define HAS_TC0220IOC 1
#define HAS_TC0100SCN 1

static volatile uint16_t *TC0260DAR = (volatile uint16_t *)0x700000;

static TC0100SCN_Layout *TC0100SCN = (TC0100SCN_Layout *)0x800000;
static TC0100SCN_Control *TC0100SCN_Ctrl = (TC0100SCN_Control *)0x820000;

static TC0220IOC_Control *TC0220IOC = (TC0220IOC_Control *)0xb00000;
static TC0360PRI_Control *TC0360PRI = (TC0360PRI_Control *)0xa00000;

static TC0200OBJ_Inst *TC0200OBJ = (TC0200OBJ_Inst *)0x900000;

static volatile uint8_t *SYT_ADDR = (volatile uint8_t *)0x200000;
static volatile uint8_t *SYT_DATA = (volatile uint8_t *)0x200002;

#elif GAME_DRIFTOUT

#define HAS_TC0260DAR 1
#define HAS_TC0360PRI 1
#define HAS_TC0430GRW 1
#define HAS_TC0220IOC 1
#define HAS_TC0100SCN 1

static volatile uint16_t *TC0260DAR = (volatile uint16_t *)0x700000;

static TC0100SCN_Layout *TC0100SCN = (TC0100SCN_Layout *)0x800000;
static TC0100SCN_Control *TC0100SCN_Ctrl = (TC0100SCN_Control *)0x820000;

static TC0430GRW_Control *TC0430GRW_Ctrl = (TC0430GRW_Control *)0x402000;
static uint16_t *TC0430GRW = (uint16_t *)0x400000;

static TC0220IOC_Control *TC0220IOC = (TC0220IOC_Control *)0xb00000;
static TC0360PRI_Control *TC0360PRI = (TC0360PRI_Control *)0xa00000;

static TC0200OBJ_Inst *TC0200OBJ = (TC0200OBJ_Inst *)0x900000;

static volatile uint8_t *SYT_ADDR = (volatile uint8_t *)0x200000;
static volatile uint8_t *SYT_DATA = (volatile uint8_t *)0x200002;

#elif GAME_DEADCONX

#define HAS_TC0260DAR 1
#define HAS_TC0360PRI 1
#define HAS_TC0480SCP 1
#define HAS_TE7750 1

static volatile uint16_t *TC0260DAR = (volatile uint16_t *)0x600000;

static TC0480SCP_Layout *TC0480SCP = (TC0480SCP_Layout *)0x400000;
static TC0480SCP_Control *TC0480SCP_Ctrl = (TC0480SCP_Control *)0x430000;

static TC0430GRW_Control *TC0430GRW_Ctrl = (TC0430GRW_Control *)0x402000;
static uint16_t *TC0430GRW = (uint16_t *)0x400000;

static TE7750_Control *TE7750 = (TE7750_Control *)0x700000;
static TC0360PRI_Control *TC0360PRI = (TC0360PRI_Control *)0x500001;

static TC0200OBJ_Inst *TC0200OBJ = (TC0200OBJ_Inst *)0x200000;

static volatile uint8_t *SYT_ADDR = (volatile uint8_t *)0xa00000;
static volatile uint8_t *SYT_DATA = (volatile uint8_t *)0xa00002;

#else

#error "No GAME_* defined"

#endif

extern volatile uint32_t vblank_count;
extern volatile uint32_t dma_count;

void wait_vblank();
void wait_dma();

#endif
