
#ifndef SIM_CORE_H
#define SIM_CORE_H

#include <functional>

void sim_init();
void sim_tick(int count = 1);
void sim_tick_until(std::function<bool()> until);
void sim_shutdown();

#endif // SIM_CORE_H
