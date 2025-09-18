#ifndef SIM_HIERARCHY_H
#define SIM_HIERARCHY_H

// Variadic macro to simplify accessing F2 core signals through sim_top wrapper
// Usage: F2_SIGNAL(color_ram, ram_l) -> sim_top__DOT__f2_inst__DOT__color_ram__DOT__ram_l
// Usage: F2_SIGNAL(cpu_word_addr) -> sim_top__DOT__f2_inst__DOT__cpu_word_addr

#define _F2_CONCAT_HELPER(...) _F2_CONCAT_IMPL(__VA_ARGS__, , , , , , , , , )
#define _F2_CONCAT_IMPL(a, b, c, d, e, f, g, h, i, ...) \
    a##__DOT__##b##__DOT__##c##__DOT__##d##__DOT__##e##__DOT__##f##__DOT__##g##__DOT__##h##__DOT__##i

#define _F2_REMOVE_TRAILING_DOTS(x) _F2_CLEAN_##x
#define _F2_CLEAN_sim_top__DOT__f2_inst__DOT____DOT____DOT____DOT____DOT____DOT____DOT____DOT__ sim_top__DOT__f2_inst
#define _F2_CLEAN_sim_top__DOT__f2_inst__DOT__a__DOT____DOT____DOT____DOT____DOT____DOT____DOT__ sim_top__DOT__f2_inst__DOT__a
#define _F2_CLEAN_sim_top__DOT__f2_inst__DOT__a__DOT__b__DOT____DOT____DOT____DOT____DOT____DOT__ sim_top__DOT__f2_inst__DOT__a__DOT__b
#define _F2_CLEAN_sim_top__DOT__f2_inst__DOT__a__DOT__b__DOT__c__DOT____DOT____DOT____DOT____DOT__ sim_top__DOT__f2_inst__DOT__a__DOT__b__DOT__c
#define _F2_CLEAN_sim_top__DOT__f2_inst__DOT__a__DOT__b__DOT__c__DOT__d__DOT____DOT____DOT____DOT__ sim_top__DOT__f2_inst__DOT__a__DOT__b__DOT__c__DOT__d
#define _F2_CLEAN_sim_top__DOT__f2_inst__DOT__a__DOT__b__DOT__c__DOT__d__DOT__e__DOT____DOT____DOT__ sim_top__DOT__f2_inst__DOT__a__DOT__b__DOT__c__DOT__d__DOT__e

// Simpler approach using token pasting
#define F2_SIGNAL_1(a) sim_top__DOT__f2_inst__DOT__##a
#define F2_SIGNAL_2(a, b) sim_top__DOT__f2_inst__DOT__##a##__DOT__##b
#define F2_SIGNAL_3(a, b, c) sim_top__DOT__f2_inst__DOT__##a##__DOT__##b##__DOT__##c
#define F2_SIGNAL_4(a, b, c, d) sim_top__DOT__f2_inst__DOT__##a##__DOT__##b##__DOT__##c##__DOT__##d
#define F2_SIGNAL_5(a, b, c, d, e) sim_top__DOT__f2_inst__DOT__##a##__DOT__##b##__DOT__##c##__DOT__##d##__DOT__##e

// Count arguments macro
#define _F2_GET_ARG_COUNT(...) _F2_GET_ARG_COUNT_IMPL(__VA_ARGS__, 5, 4, 3, 2, 1, 0)
#define _F2_GET_ARG_COUNT_IMPL(_1, _2, _3, _4, _5, N, ...) N

// Main macro that dispatches to the correct arity version
#define F2_SIGNAL(...) _F2_SIGNAL_DISPATCH(_F2_GET_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)
#define _F2_SIGNAL_DISPATCH(N, ...) _F2_SIGNAL_CONCAT(F2_SIGNAL_, N)(__VA_ARGS__)
#define _F2_SIGNAL_CONCAT(a, b) a##b

#define G_F2_SIGNAL(...) g_sim_core.top->rootp->_F2_SIGNAL_DISPATCH(_F2_GET_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)

#endif // SIM_HIERARCHY_H
