/*
 Copyright (C) 2011 Georgia Institute of Technology

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

/*
 * Persistent sodium channel
 */

#include <default_gui_model.h>
#include <math.h>
#include <string>

class g_Na_p : public DefaultGUIModel
{

public:

  g_Na_p(void);
  virtual
  ~g_Na_p(void);

  virtual void
  execute(void);

protected:

  virtual void
  update(DefaultGUIModel::update_flags_t);

private:

  double seed;
  bool noise_on;
  double vna;
  double gnap_max;
  unsigned int nnap;
  double pmax;
  double taup;
  double v_half;
  double dv;
  double rand_num[2];
  double dt;

};
