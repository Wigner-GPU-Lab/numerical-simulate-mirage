#include  <boost/program_options.hpp>
#include "nr3.h"
#include "stepper.h"
#include "stepperdopr853.h"
#include "odeint.h"
#include "OdeSolver.h"
#include "StepperDormandPrice853.h"

// g++ eikonal.cpp -Inumrec -o eikonal -ggdb -lboost_program_options

// These two are for conventional asphalt
double refract(double const aH, double const aTempAmb, double const aTempDiff) {
  auto celsius = aTempAmb + 0.018 + aTempDiff * std::exp(-aH * 10.08); // Toled vettem.
  return 1.0 + 7.86e-4 * 101 / (celsius + 273.15);
}

double refractDiff(double const aH, double const aTempAmb, double const aTempDiff) {
  auto t = aTempAmb + aTempDiff * std::exp(-10.08 * aH) + 273.168;
  return aTempDiff * 0.800211 * std::exp(-10.08 * aH) / t / t;
}


class Eikonal final {
public:
  static constexpr double cgC = 299792458; // m/2
  
private:
  double mTempAmb;
  double mTempDiff;

public:
  Eikonal(double const aTempAmb, double const aTempDiff) : mTempAmb(aTempAmb), mTempDiff(aTempDiff) {}

  void operator() (const double/* aS*/, VecDoub_I const &aY, VecDoub_O &aDyds) {
    double n    = refract(aY[2], mTempAmb, mTempDiff);
    double v    = cgC / n;
//    double dvdz = -v / n * refractDiff(aY[2]);

    aDyds[0] = v * aY[3];
    aDyds[1] = v * aY[4];
    aDyds[2] = v * aY[5];
    aDyds[3] = 0.0;
    aDyds[4] = 0.0;
    aDyds[5] = refractDiff(aY[2], mTempAmb, mTempDiff) / v / n;
    //aDyds[5] = -1.0 / v / v * dvdz;
  }
};

template<typename tReal>
class Eikonal2 final {
public:
  static constexpr double cgC = 299792458; // m/2
  static constexpr uint32_t csNvar = 6u;
  using Real                       = tReal;
  using Variables                  = std::array<Real, csNvar>;
  
private:
  double mTempAmb;
  double mTempDiff;

public:
  Eikonal2(double const aTempAmb, double const aTempDiff) : mTempAmb(aTempAmb), mTempDiff(aTempDiff) {}

  void operator() (const double/* aS*/, std::array<Real, csNvar> const &aY, std::array<Real, csNvar> &aDyds) {
    double n    = refract(aY[2], mTempAmb, mTempDiff);
    double v    = cgC / n;
//    double dvdz = -v / n * refractDiff(aY[2]);

    aDyds[0] = v * aY[3];
    aDyds[1] = v * aY[4];
    aDyds[2] = v * aY[5];
    aDyds[3] = 0.0;
    aDyds[4] = 0.0;
    aDyds[5] = refractDiff(aY[2], mTempAmb, mTempDiff) / v / n;
    //aDyds[5] = -1.0 / v / v * dvdz;
  }
};

void comp(double aDir, double aDist, double aHeight, double aStep1, double aStepMin, double aTempAmb, double aTempDiff, double aTolAbs, double aTolRel, double aTarget) {
  constexpr uint32_t cNvar = 6u;
  VecDoub yStart(cNvar);
  yStart[0] = 0.0;
  yStart[1] = 0.0;
  yStart[2] = aHeight;
  auto u = refract(aHeight, aTempAmb, aTempDiff) / Eikonal::cgC;
  yStart[3] = u * std::cos(aDir / 180.0 * 3.1415926539);
  yStart[4] = 0.0;
  yStart[5] = u * std::sin(aDir / 180.0 * 3.1415926539);
  Output out(100);
  Eikonal eikonal(aTempAmb, aTempDiff);
  Odeint<StepperDopr853<Eikonal>> ode(yStart, 0.0, aDist, aTolAbs, aTolRel, aStep1, aStepMin, out, eikonal);
  ode.integrate();
std::cout << "==========================================================\n";
  std::array<double, cNvar> yStart2;
  yStart2[0] = 0.0;
  yStart2[1] = 0.0;
  yStart2[2] = aHeight;
  yStart2[3] = u * std::cos(aDir / 180.0 * 3.1415926539);
  yStart2[4] = 0.0;
  yStart2[5] = u * std::sin(aDir / 180.0 * 3.1415926539);
  Eikonal2<double> eikonal2(aTempAmb, aTempDiff);
  OdeSolver<StepperDormandPrice853<Eikonal2<double>>> ode2(yStart2, 0.0, aDist, aTolAbs, aTolRel, aStep1, aStepMin, eikonal2);
  auto result = ode2.solve([aTarget](std::array<double, cNvar> const& aY){ 
std::cout << "judge x " << aY[0] <<"\n\n";
return aY[0] >= aTarget;
}, 0.0001);
std::cout << "result y: "; for(auto i : result) std::cout << i << ' '; std::cout << '\n';
  std::cout << "x2=[0, " << result[0] << "]\n";
  std::cout << "z2=[1, " << result[2] << "]\n";
  std::cout << "x=[";
  for (Int i=0; i<out.count; i++) {
    std::cout << out.ysave[0][i] << (i < out.count - 1 ? ", " : "];\n");
  }
  std::cout << "z=[";
  for (Int i=0; i<out.count; i++) {
    std::cout << out.ysave[2][i] << (i < out.count - 1 ? ", " : "];\n");
  }
  std::cout << "\n";
}

int main(int aArgc, char **aArgv) {
  boost::program_options::options_description desc("Usage:");
  desc.add_options()
                    ("dir", boost::program_options::value<double>(), "start direction (degrees), neg is down, 0 horizontal [0]")
                    ("dist", boost::program_options::value<double>(), "horizontal distance to track [2000]")
                    ("height", boost::program_options::value<double>(), "start height (m) [1]")
                    ("help", "produce this message")
                    ("step1", boost::program_options::value<double>(), "initial step size [0.01]")
                    ("stepmin", boost::program_options::value<double>(), "minimal step size [0.0]")
                    ("tempamb", boost::program_options::value<double>(), "ambient temperature (Celsius) [20]")
                    ("tempdiff", boost::program_options::value<double>(), "temperature rise next to asphalt compared to ambient (Celsius) [6.37]")
                    ("tolabs", boost::program_options::value<double>(), "absolute tolerance [1e-3]")
                    ("tolrel", boost::program_options::value<double>(), "relative tolerance [1e-3]")
                    ("xyz", boost::program_options::value<double>(), "custom [0]");
  boost::program_options::variables_map varMap;
  boost::program_options::store(boost::program_options::parse_command_line(aArgc, aArgv, desc), varMap);
  boost::program_options::notify(varMap);
  if (varMap.count("help")) {
    cout << desc << "\n";
  }
  else {
    double dir = (varMap.count("dir") ? varMap["dir"].as<double>() : 0.0);
    double dist = (varMap.count("dist") ? varMap["dist"].as<double>() : 2000.0);
    double height = (varMap.count("height") ? varMap["height"].as<double>() : 1.0);
    double step1 = (varMap.count("step1") ? varMap["step1"].as<double>() : 0.01);
    double stepMin = (varMap.count("stepmin") ? varMap["stepmin"].as<double>() : 0.0);
    double tempAmb = (varMap.count("tempamb") ? varMap["tempamb"].as<double>() : 20);
    double tempDiff = (varMap.count("tempdiff") ? varMap["tempdiff"].as<double>() : 6.37);
    double tolAbs = (varMap.count("tolabs") ? varMap["tolabs"].as<double>() : 0.001);
    double tolRel = (varMap.count("tolrel") ? varMap["tolrel"].as<double>() : 0.001);
    double target = (varMap.count("xyz") ? varMap["xyz"].as<double>() : 0.0);
    comp(dir, dist, height, step1, stepMin, tempAmb, tempDiff, tolAbs, tolRel, target);
  }
  return 0;
}
