#include "Eikonal.h"
#include "3dGeomUtil.h"
#include "OdeSolverGsl.h"
#include "RungeKuttaRayBending.h"
#include "mathUtil.h"
#include "CLI.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>


struct MoreParameters {
  Eikonal::EarthForm mEarthForm;
  double             mEarthRadius;
  Eikonal::Model     mMode;
  double             mTempAmb;
  double             mTempBase;
  double             mDir;
  double             mCamCenter;
  double             mDist;
  uint32_t           mSamples;
  bool               mSilent;
};

RungeKuttaRayBending::Result comp1(RungeKuttaRayBending::Parameters const& aParameters, MoreParameters const& aMore) {

  Eikonal eikonal(aMore.mEarthForm, aMore.mEarthRadius, aMore.mMode, aMore.mTempAmb, aMore.mTempAmb, aMore.mTempAmb, aMore.mTempBase);
  RungeKuttaRayBending rk(aParameters, eikonal);
  Vertex start(0.0, aMore.mCamCenter, 0.0);
  Vector dir(std::cos(aMore.mDir / 180.0 * cgPi), std::sin(aMore.mDir / 180.0 * cgPi), 0.0);
  RungeKuttaRayBending::Result solution;
  if(aMore.mDist == 0.0) {
    solution.mValid = true;
    solution.mValue[0] = 0.0;
    solution.mValue[1] = aMore.mCamCenter;
    solution.mValue[2] = 0.0;
  }
  else {
    solution = rk.solve4x(start, dir, aMore.mDist);
  }
  return solution;
}

void comp(std::string const& aPrefix, RungeKuttaRayBending::Parameters const& aParameters, MoreParameters const& aMore, bool aNeedXd) {
  std::vector<Vertex> stuff;
  std::ofstream out(aPrefix + "values.txt");
  auto end = aMore.mDist * (1.0 + 0.5 / aMore.mSamples);
  auto more = aMore;
  for(more.mDist = 0.0; more.mDist <= end; more.mDist += aMore.mDist / aMore.mSamples) {
    RungeKuttaRayBending::Result solution = comp1(aParameters, more);
    if(solution.mValid) {
      stuff.push_back(solution.mValue);
      out << std::setprecision(10) << solution.mValue[0] << '\t' << std::setprecision(10) << solution.mValue[1] << '\n';
    }
  }
  if(!aMore.mSilent) {
    if(aNeedXd) {
      if(aMore.mEarthForm == Eikonal::EarthForm::cRound) {
        std::cout << "d=[";
        for (int i = 0; i < stuff.size(); ++i) {
          std::cout << std::setprecision(10) << std::sqrt(aMore.mEarthRadius * aMore.mEarthRadius - stuff[i][0] * stuff[i][0]) - aMore.mEarthRadius << (i < stuff.size() - 1 ? ", " : "];\n");
        }
      }
      else {} // nothing to do
      std::cout << "x=[";
      for (int i = 0; i < stuff.size(); ++i) {
        std::cout << std::setprecision(10) << stuff[i][0] << (i < stuff.size() - 1 ? ", " : "];\n");
      }
    }
    else {} // nothing to do
    std::cout << aPrefix + "y=[";
    for (int i = 0; i < stuff.size(); ++i) {
      std::cout << std::setprecision(10) << stuff[i][1] << (i < stuff.size() - 1 ? ", " : "];\n");
    }
  }
  else {} // nothing to do
}

enum class CliResult : uint8_t {
  cOk          = 0u,
  cCliError    = 1u,
  cCliHelp     = 2u,
  cParamError  = 3u
};

std::tuple<CliResult, RungeKuttaRayBending::Parameters, MoreParameters, std::string, std::string, std::string> parse(int aArgc, char **aArgv) {
  CliResult result = CliResult::cOk;
  RungeKuttaRayBending::Parameters parameters;
  MoreParameters more;

  CLI::App opt{"Usage"};
  std::string nameBase = "water";
  opt.add_option("--base", nameBase, "base type (conventional / porous / water) [water]");
  more.mCamCenter = 1.1;
  opt.add_option("--camCenter", more.mCamCenter, "start height (m) [1.1]");
  more.mDir = std::nan("");
  opt.add_option("--dir", more.mDir, "start direction, neg downwards (degrees) [computed to touch surface]");
  more.mDist = 1000.0;
  opt.add_option("--dist", more.mDist, "horizontal distance to travel (m) [1000]");
  std::string nameForm = "round";
  opt.add_option("--earthForm", nameForm, "Earth form (flat / round) [round]");
  double rawRadius = 6371.0;
  opt.add_option("--earthRadius", rawRadius, "Earth radius (km) [6371.0]");
  parameters.mMaxCosDirChange = 0.99999999999;
  opt.add_option("--maxCosDirChange", parameters.mMaxCosDirChange, "Maximum of cos of direction change to reset big step [0.99999999999]");
  more.mSamples = 100;
  opt.add_option("--samples", more.mSamples, "number of samples on ray [100]");
  more.mSilent = true;
  opt.add_option("--silent", more.mSilent, "surpress parameter echo (true, false) [true]");
  parameters.mStep1 = 0.01;
  opt.add_option("--step1", parameters.mStep1, "initial step size (m) [0.01]");
  parameters.mStepMin = 1e-7;
  opt.add_option("--stepMin", parameters.mStepMin, "maximal step size (m) [1e-7]");
  parameters.mStepMax = 22.2;
  opt.add_option("--stepMax", parameters.mStepMax, "maximal step size (m) [22.2]");
  std::string nameStepper = "RungeKuttaFehlberg45";
  opt.add_option("--stepper", nameStepper, "stepper type (RungeKutta23 / RungeKuttaClass4 / RungeKuttaFehlberg45 / RungeKuttaCashKarp45 / RungeKuttaPrinceDormand89 / BulirschStoerBaderDeuflhard) [RungeKuttaFehlberg45]");
  more.mTempAmb = std::nan("");
  opt.add_option("--tempAmb", more.mTempAmb, "ambient temperature (Celsius) [20 for conventional, 38.5 for porous, 10 for water]");
  more.mTempBase = 13.0;
  opt.add_option("--tempBase", more.mTempBase, "base temperature, only for water (Celsius) [13]");
  parameters.mTolAbs = 0.001;
  opt.add_option("--tolAbs", parameters.mTolAbs, "absolute tolerance (m) [1e-3]");
  parameters.mTolRel = 0.001;
  opt.add_option("--tolRel", parameters.mTolRel, "relative tolerance (m) [1e-3]");
  try {
    opt.parse(aArgc, aArgv);

    if(nameBase == "conventional") {
      more.mMode = Eikonal::Model::cConventional;
    }
    else if(nameBase == "porous") {
      more.mMode = Eikonal::Model::cPorous;
    }
    else if(nameBase == "water") {
      more.mMode = Eikonal::Model::cWater;
    }
    else {
      std::cerr << "Illegal base value: " << nameBase << '\n';
      result = CliResult::cParamError;
    }

    if(nameForm == "flat") {
      more.mEarthForm = Eikonal::EarthForm::cFlat;
    }
    else if(nameForm == "round") {
      more.mEarthForm = Eikonal::EarthForm::cRound;
    }
    else {
      std::cerr << "Illegal Earth form value: " << nameForm << '\n';
      result = CliResult::cParamError;
    }

    more.mEarthRadius = rawRadius * 1000.0;

    if(nameStepper == "RungeKutta23") {
      parameters.mStepper = StepperType::cRungeKutta23;
    }
    else if(nameStepper == "RungeKuttaClass4") {
      parameters.mStepper = StepperType::cRungeKuttaClass4;
    }
    else if(nameStepper == "RungeKuttaFehlberg45") {
      parameters.mStepper = StepperType::cRungeKuttaFehlberg45;
    }
    else if(nameStepper == "RungeKuttaCashKarp45") {
      parameters.mStepper = StepperType::cRungeKuttaCashKarp45;
    }
    else if(nameStepper == "RungeKuttaPrinceDormand89") {
      parameters.mStepper = StepperType::cRungeKuttaPrinceDormand89;
    }
    else if(nameStepper == "BulirschStoerBaderDeuflhard") {
      parameters.mStepper = StepperType::cBulirschStoerBaderDeuflhard;
    }
    else {
      std::cerr << "Illegal stepper value: " << nameStepper << '\n';
      result = CliResult::cParamError;
    }

    if(std::isnan(more.mTempAmb)) {
      more.mTempAmb = (more.mMode == Eikonal::Model::cConventional ? 20.0 :
              (more.mMode == Eikonal::Model::cPorous ? 38.5 : 10.0));
    }
    else {} // nothing to do
  }
  catch(CLI::RuntimeError &e) {
    std::cout << e.get_exit_code() << '\n';
    result = CliResult::cCliError;
  }
  catch(CLI::CallForHelp &) {
    std::cout << opt.help() << '\n';
    result = CliResult::cCliHelp;
  }
  catch(CLI::CallForAllHelp &) {
    std::cout << opt.help("", CLI::AppFormatMode::All) << '\n';
    result = CliResult::cCliHelp;
  }
  catch(CLI::CallForVersion &e) {
    std::cout << e.what() << '\n';
    result = CliResult::cCliHelp;
  }

  parameters.mDistAlongRay    = more.mDist * 2.0;
  return std::make_tuple(result, parameters, more, nameBase, nameForm, nameStepper);
}

bool resolveCriticalIfNeeded(RungeKuttaRayBending::Parameters const& aParameters, MoreParameters &aMore) {
  double const cTolerance = 1e-6;
  auto parameters = aParameters;
  auto more = aMore;
  parameters.mTolAbs = cTolerance;
  parameters.mTolRel = cTolerance;
  more.mDir = 0.0;

  auto solution = comp1(parameters, more);
  if(solution.mValid) {
    if(std::isnan(aMore.mDir)) {
      aMore.mDir = binarySearch(-45, 0.0, cTolerance, [&parameters, &more](auto const angle){
        more.mDir = angle;
        auto solution = comp1(parameters, more);
        return solution.mValid;
      }) + cTolerance;
    }
    else {} // nothing to do
  }
  else {} // nothing to do

  return solution.mValid;
}

double calculateMirrorDirection(RungeKuttaRayBending::Parameters const& aParameters, MoreParameters const& aMore) {
  double const cTolerance = 1e-6;
  auto parameters = aParameters;
  auto more = aMore;
  parameters.mTolAbs = cTolerance;
  parameters.mTolRel = cTolerance;

  more.mDir = 0.0;
  bool   downwardsPrev = true;
  double delta = -0.1;

  while(std::abs(delta) > cTolerance) {
    more.mDir += delta;
    auto solution = comp1(parameters, more);
    bool downwardsNow = (solution.mDirection(1) < 0);
    if(downwardsPrev != downwardsNow) {
      delta /= -3.0;
      downwardsPrev = downwardsNow;
    }
    else {} // nothing to do
  }

  return more.mDir;
}

void dump(RungeKuttaRayBending::Parameters const& aParameters, MoreParameters const& aMore, double const aMirrorDirection, std::string const& aNameBase, std::string const& aNameForm, std::string const& aNameStepper) {
  if(!aMore.mSilent) {
    std::cout << "base type:                               .  .  .  " << aNameBase << ' ' << static_cast<int>(aMore.mMode) << '\n';
    std::cout << "camera height (m):                                " << aMore.mCamCenter << '\n';
    std::cout << "start direction, neg downwards (degrees):         " << aMore.mDir << '\n';
    std::cout << "horizontal distance to travel (m):    .  .  .  .  " << aMore.mDist << '\n';
    std::cout << "Earth form:                                       " << aNameForm << ' ' << static_cast<int>(aMore.mEarthForm) << '\n';
    std::cout << "Earth radius (km):                                " << aMore.mEarthRadius / 1000.0 << '\n';
    std::cout << "max of cos of direction change to reset big step: " << std::setprecision(17) << aParameters.mMaxCosDirChange << '\n';
    std::cout << "number of samples on ray:                         " << aMore.mSamples << '\n';
    std::cout << "initial step size (m):                            " << aParameters.mStep1 << '\n';
    std::cout << "minimal step size (m): .  .  .  .  .  .  .  .  .  " << aParameters.mStepMin << '\n';
    std::cout << "maximal step size (m):                            " << aParameters.mStepMax << '\n';
    std::cout << "stepper type:                                     " << aNameStepper << ' ' << static_cast<int>(aParameters.mStepper) << '\n';
    std::cout << "ambient temperature (Celsius):              .  .  " << aMore.mTempAmb << '\n';
    std::cout << "base temperature, only for water (Celsius):       " << aMore.mTempBase << '\n';
    std::cout << "absolute tolerance (m):                           " << aParameters.mTolAbs << '\n';
    std::cout << "relative tolerance (m):               .  .  .  .  " << aParameters.mTolRel << '\n';
  }
  else {} // nothing to do
  std::cout << "mirror direction (computed) (degrees):            " << aMirrorDirection << '\n';
}

int main(int aArgc, char **aArgv) {
  auto[result, parameters, more, nameBase, nameForm, nameStepper] = parse(aArgc, aArgv);

  bool valid = (result == CliResult::cOk && resolveCriticalIfNeeded(parameters, more));

  if(valid) {
    double mirrorDirection = calculateMirrorDirection(parameters, more);
    dump(parameters, more, mirrorDirection, nameBase, nameForm, nameStepper);
    comp("crit", parameters, more, true);
    more.mDir = mirrorDirection;
    comp("mirr", parameters, more, false);
  }
  else {
    if(result == CliResult::cOk) {
      std::cout << "Can't compute critical ray, increase --dist.\n";
    }
    else if(result == CliResult::cParamError) {
      std::cout << "Wrong parameters.\n";
    }
    else {} // nothing to do
  }
  return 0;
}
