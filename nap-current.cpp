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

#include <nap-current.h>
#include <math.h>
#include <QtGui>

extern "C" Plugin::Object *
createRTXIPlugin(void)
{
  return new g_Na_p();
}

static DefaultGUIModel::variable_t vars[] =
  {
    { "Vm", "Membrane Voltage (in mV)", DefaultGUIModel::INPUT, },
    { "Iapp", "Injected current (in pA)", DefaultGUIModel::OUTPUT, },
    { "Random Seed", "Seed for random number generator",
        DefaultGUIModel::PARAMETER | DefaultGUIModel::INTEGER, },
    { "Noise On", "1 for stochastic channel 0 for deterministic",
        DefaultGUIModel::PARAMETER | DefaultGUIModel::UINTEGER, },
    { "Vna", "Sodium reversal potential (mV)", DefaultGUIModel::PARAMETER
        | DefaultGUIModel::DOUBLE, },
    { "Gnap_max", "Maximum channel conductance (ns)",
        DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
    { "Nnap", "# of persistent Na channels", DefaultGUIModel::PARAMETER
        | DefaultGUIModel::INTEGER, },
    { "pmax", "Maximum fraction of open channels", DefaultGUIModel::PARAMETER
        | DefaultGUIModel::DOUBLE, },
    { "tau_p", "Persistent Na channel time constant",
        DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
    { "Vp_half", "PNa half activation voltage (mV)", DefaultGUIModel::PARAMETER
        | DefaultGUIModel::DOUBLE, },
    { "dV", "related to slope of activation curve (mV)",
        DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },

  };

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

g_Na_p::g_Na_p(void) :
  DefaultGUIModel("Persistent Sodium", ::vars, ::num_vars),
      seed(13), noise_on(0), vna(55.0), gnap_max(0), nnap(1200), pmax(1.0),
      taup(0.15), v_half(-45.9), dv(3.2)
{
  setWhatsThis(
      "<p><b>Persistent Sodium Conductance:</b><br>This module creates a persistent sodium conductance.</p>");
  dt = RT::System::getInstance()->getPeriod() * 1e-6;

  rand_num[0] = 1;
  rand_num[1] = 1;
  createGUI(vars, num_vars);
  update(INIT);
  refresh();
  QTimer::singleShot(0, this, SLOT(resizeMe()));
}

g_Na_p::~g_Na_p(void)
{
}

void
g_Na_p::execute(void)
{

  static double p = 0;
  double Vm = input(0) * 1e3 - 10; //Assuming 10 mV junction potential
  double Iout;
  double t_ratio = dt / taup;
  double p_inf = pmax / (1 + exp((v_half - Vm) / dv));
  double dp_det = (p_inf - p) * (1 - exp(-t_ratio));
  double gnap;
  static int rand_flag = 0;
  double dp_rnd, dp_rnd_x1, dp_rnd_x2;

  if (noise_on)
    {
      // Algorithm to call random() once per step
      rand_flag = (rand_flag + 1) % 2;
      if (rand_flag)
        {
          rand_num[0] = (double) rand() / (double) RAND_MAX;
          dp_rnd_x1 = cos(2 * M_PI * rand_num[0]);
        }
      else
        {
          rand_num[1] = (double) rand() / (double) RAND_MAX;
          dp_rnd_x1 = sin(2 * M_PI * rand_num[0]);
        }
      dp_rnd_x2 = -2 * t_ratio * (p_inf + p - 2 * p_inf * p) / nnap;
      dp_rnd = dp_rnd_x1 * sqrt(dp_rnd_x2 * log(rand_num[1]));
    }
  else
    {
      dp_rnd = 0;
    }

  p = p + dp_det + dp_rnd;
  if (p > pmax)
    p = pmax;
  if (p < 0)
    p = 0.000;

  gnap = gnap_max * p; // nS

  Iout = gnap * (vna - input(0)) * 1e-12; // [=] nS * mV = pA
  output(0) = Iout;

}

void
g_Na_p::update(DefaultGUIModel::update_flags_t flag)
{
  switch (flag)
    {
  case INIT:
    setParameter("Random Seed", QString::number(seed));
    setParameter("Noise On", QString::number(noise_on));
    setParameter("Vna", QString::number(vna));
    setParameter("Gnap_max", QString::number(gnap_max));
    setParameter("Nnap", QString::number(nnap));
    setParameter("pmax", QString::number(pmax));
    setParameter("tau_p", QString::number(taup));
    setParameter("Vp_half", QString::number(v_half));
    setParameter("dV", QString::number(dv));
    break;
  case MODIFY:
    seed = getParameter("Random Seed").toDouble();
    noise_on = getParameter("Noise On").toUInt();
    vna = getParameter("Vna").toDouble();
    gnap_max = getParameter("Gnap_max").toDouble();
    nnap = getParameter("Nnap").toUInt();
    pmax = getParameter("pmax").toDouble();
    taup = getParameter("tau_p").toDouble();
    v_half = getParameter("Vp_half").toDouble();
    dv = getParameter("dV").toDouble();

    break;
  case PERIOD:
    dt = RT::System::getInstance()->getPeriod() * 1e-6;
  default:
    break;
    }
}
