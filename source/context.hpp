// A context defines a locale in which reactions take place and species 
//   reside
#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "specie.hpp"
#include "concentration_level.hpp"
#include <array>


class Context {
  //FIXME - want to make this private at some point
 public:
  const int _cell;
  simulation& _simulation;
  double _avg;
  Context(simulation& sim, int cell) : _simulation(sim),_cell(cell) { }
  RATETYPE calculateNeighbourAvg(specie_id sp) const;
  void updateCon(const std::array<RATETYPE, NUM_SPECIES>& rates);
  const std::array<RATETYPE, NUM_SPECIES> calculateRatesOfChange();
  RATETYPE getCon(specie_id sp, int delay = 0) const {
    // FIXME: calculate an actual time, not just using the delay, and make sure the indexes are 
    // in the right order
      int modified_step = _simulation._baby_j[sp] - delay;
    return _simulation._baby_cl[sp][modified_step][_cell];
  }
  RATETYPE getRate(int reaction) const {
        return _simulation._rates[reaction][_cell];
    }
  RATETYPE cal_avgpd(specie_id sp) const;
  RATETYPE cal_transcription(specie_id sp) const;
  RATETYPE cal_tdelta(specie_id sp) const;
  RATETYPE cal_tdimer(specie_id sp) const;
};

#endif // CONTEXT_HPP
