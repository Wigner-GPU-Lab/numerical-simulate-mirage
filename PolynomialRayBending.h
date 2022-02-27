#ifndef POLYNOMIALRAYBENDING
#define POLYNOMIALRAYBENDING

#include "3dGeomUtil.h"
#include "mathUtil.h"
#include <functional>
#include <numeric>
#include <memory>
#include <random>
#include <deque>


// These calculations do not take relative humidity in account, since it has less, than 0.5% the effect on air refractive index as temperature and pressure.
class PolynomialRayBending final {
public:
  struct DispDepth {
    double   mDisp;
    double   mDepth;
    uint32_t mIter;
    DispDepth(double const aDisp, double const aDepth, uint32_t const aIter) : mDisp(aDisp), mDepth(aDepth), mIter(aIter) {}
  };

  static constexpr double   csCelsius2kelvin                = 273.15;

private:
  static constexpr double   csTempAmbient                   = 297.5;                                        // Kelvin
  static constexpr uint32_t csTempProfilePointCount         =   8u;
  static constexpr uint32_t csTempProfileDegree             =   4u;
  static constexpr double   csTplate[csTempProfilePointCount]      = { 336.7, 331.7, 326.7, 321.7, 316.7, 311.7, 306.7, 301.7 };  // Kelvin
  static constexpr double   csHeightLimit[csTempProfilePointCount] = { 0.25,  0.25,  0.25,  0.3,   0.35,  0.35,  0.4,   0.4   };  // cm
  static constexpr double   csB[csTempProfilePointCount]           = { 0.048, 0.041, 0.035, 0.030, 0.024, 0.018, 0.012, 0.006 };
  static constexpr double   csDelta[csTempProfilePointCount]       = { 1.4,   1.4,   1.5,   1.5,   1.6,   1.6,   1.6,   1.6   };
  static constexpr double   csDeltaFallback                 = 1.2;

  static constexpr uint32_t csRayTraceCountAsphalt          = 101u;
  static constexpr uint32_t csRayTraceCountBending          = 101u;
  static constexpr uint32_t csPolynomDegreeHeight           =   7u;
  static constexpr uint32_t csPolynomDegreeDirection        =   7u;
  static constexpr uint32_t csPolynomDegreeHorizDisp        =   7u;
  static constexpr uint32_t csSamplePointsOnRay             =  15u;
  static constexpr double   csRelativeRandomRadius          =   0.25;

  static constexpr double   csRelativeHumidityPercent       =  50.0;
  static constexpr double   csAtmosphericPressureKpa        = 101.0;
  static constexpr double   csLayerDeltaTemp                =   0.00005;
  static constexpr double   csInitialHeight                 =   1.0;      // TODO consider if this can be constant for larger plate temperatures.
  static constexpr double   csMinimalHeight                 =   0.001;
  static constexpr double   csAlmostVertical                =   0.01;
  static constexpr double   csAlmostHorizontal              =   (cgPi / 2.0) * 0.9999;
  static constexpr double   csAsphaltRayAngleLimit          =   (cgPi / 4.0);
  static constexpr double   csEpsilon                       =   0.00001;

public: // TODO private when ready
  // Used to gather data from one half ray bending trace.
  struct RawSample final {
    double mHeight;
    double mHorizDisp;      // starting from mHeight to the next point
    double mSinInclination;
    RawSample(double const aHeight, double const aHorizDisp, double const aSinInclination) : mHeight(aHeight), mHorizDisp(aHorizDisp), mSinInclination(aSinInclination) {}
  };

  // Used to gather data from all the traces
  struct Sample final {
    double mHeight;
    double mHorizDisp;      // Cumulative, from starting point.
    double mAngleFromHoriz; // 0 is horizontal, negative points downwards, positive upwards.
    Sample(double const aHeight, double const aHorizDisp, double const aAngleFromHoriz) : mHeight(aHeight), mHorizDisp(aHorizDisp), mAngleFromHoriz(aAngleFromHoriz) {}
  };

  // Used to generate Vandermonde matrices and polynomial fit
  struct Relation final {
    double mStartHeight;
    double mStartAngleFromHoriz;
    double mHorizDisp;
    double mEndHeight;
    double mEndAngleFromHoriz;
    Relation(double const aStartHeight, double const aStartAngleFromHoriz, double const aHorizDisp, double const aEndHeight, double const aEndAngleFromHoriz)
      : mStartHeight(aStartHeight)
      , mStartAngleFromHoriz(aStartAngleFromHoriz)
      , mHorizDisp(aHorizDisp)
      , mEndHeight(aEndHeight)
      , mEndAngleFromHoriz(aEndAngleFromHoriz) {}
  };

  struct Gather final {
    bool                  mAsphalt;
    std::deque<RawSample> mCollection;
  };

  struct Intermediate final {
    bool                mAsphalt;
    std::vector<Sample> mCollection;
  };

private:
  struct Static final {
    std::unique_ptr<PolynomApprox> mHeightLimit;
    std::unique_ptr<PolynomApprox> mB;
    std::unique_ptr<PolynomApprox> mDelta;

    Static();
  };

  double mTempDiffSurface;
  double mHeightLimit;
  double mB;
  double mDelta;
  double mCriticalInclination;

  mutable std::minstd_rand                       mRandomEngine;
  mutable std::uniform_real_distribution<double> mRandomDistribution;

public:
  PolynomialRayBending(double const aTempDiffSurface); // TODO this should accept ambient temperature in the final version.

  static double getWorkingHeight()                                   { return csInitialHeight; }
  static double getAmbientTemp()    /* Kelvin */                     { return csTempAmbient; }
  double getCriticalInclination()                              const { return mCriticalInclination; }
  double getTempRiseAtHeight(double const aHeight)             const;  // delta Celsius and meter
  double getHeightAtTempRise(double const aTempRise)           const;  // delta Celsius and meter
  double getRefractionAtTempRise(double const aTempRise)       const;  // delta Celsius
  double getRefractionAtHeight(double const aHeight)           const { return getRefractionAtTempRise(getTempRiseAtHeight(aHeight)); }

private:
  void initReflection();
  Gather getReflectionDispDepth(double const aInclination) const;

public:  // TODO private when ready
  static Intermediate   toRayPath(Gather const aRaws);
  void                  addForward(std::deque<Relation> &aCollector, std::vector<Sample> const &aLot) const;
  void                  addReverse(std::deque<Relation> &aCollector, std::vector<Sample> const &aLot) const;
  std::vector<uint32_t> getRandomIndices(uint32_t const aFromCount, uint32_t const aChosenCount) const;
};

#endif
