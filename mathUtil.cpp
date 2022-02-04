#include "mathUtil.h"
#include <numeric>

#include <iostream> // TODO remove


double signum(double const aValue) {
  return std::copysign(1.0f, aValue);
}

double binarySearch(double const aLower, double const aUpper, double const aEpsilon, std::function<double(double)> aLambda) {
//std::cout << "--- " << aLower << "    " << aUpper << '\n';
  double signLower = signum(aLambda(aLower));
  double lower = aLower;
  double upper = aUpper;
  double result = (lower + upper) / 2.0f;
  while(upper - lower > aEpsilon) {
    double now = signum(aLambda(result));
    if(now == signLower) {
//std::cout << "++ " << now << "   " << result << '\n';
      lower = result;
    }
    else {
//std::cout << "-- " << now << "   " << result << '\n';
      upper = result;
    }
    result = (lower + upper) / 2.0f;
  }
//std::cout << "===========================\n";
  return result;
}


PolynomApprox::PolynomApprox(double const* const aSamplesX, double const* const aSamplesY, uint32_t const aSampleCount, uint32_t const aDegreeMinus, uint32_t const aDegreePlus)
  : mDegreeMinus(aDegreeMinus)
  , mCoeffCount(aDegreeMinus + 1u + aDegreePlus)
  , mCoefficients(mCoeffCount) {
  Eigen::MatrixXd fitA(aSampleCount, mCoeffCount);
  Eigen::VectorXd fitB = Eigen::VectorXd::Map(aSamplesY, aSampleCount);

  if(aSampleCount >= mCoeffCount && mCoeffCount > 1u) {
    auto [itMin, itMax] = std::minmax_element(aSamplesX, aSamplesX + aSampleCount);
    mXmin = *itMin;
    mSpanOriginal = *itMax - mXmin;
    auto span = getXspan(mCoeffCount - 1u);
    if(mDegreeMinus == 0u) {
      mSpanFactor = span * 2.0;
      mSpanStart  = -span;
    }
    else {
      mSpanFactor = span / 2.0;
      mSpanStart  = span / 2.0;
    }
std::cout << "so: " << mSpanOriginal << " sf: " << mSpanFactor << " ss: " << mSpanStart << '\n';
    for(uint32_t i = 0u; i < aSampleCount; ++i) {
      for(uint32_t j = 0u; j < mCoeffCount; ++j) {
        fitA(i, j) = ::pow(normalize(aSamplesX[i]), static_cast<int32_t>(j) - static_cast<int32_t>(aDegreeMinus));
std::cout << fitA(i, j) << "   ";
      }
std::cout << '\n';
    }
    //mCoefficients = (fitA.transpose() * fitA).completeOrthogonalDecomposition().pseudoInverse() * fitA.transpose() * fitB;
    mCoefficients = (fitA.transpose() * fitA).inverse() * fitA.transpose() * fitB;
for(int i = 0; i < mCoeffCount; ++i) {
  std::cout << "coeff " << i << ": " << mCoefficients(i) << '\n';
}
    auto diffs = 0.0f;
    auto desireds = 0.0f;
    for(uint32_t i = 0u; i < aSampleCount; ++i) {
      auto x = aSamplesX[i];
      auto desired = aSamplesY[i];
      auto y = eval(x);
std::cout << y << ", ";
      auto diff = desired - y;
      diffs += diff * diff;
      desireds += desired * desired;
    }
    mRrmsError = (desireds > 0.0f ? ::sqrt(diffs / desireds / aSampleCount) : 0.0f);
std::cout << "\n error: " << mRrmsError << '\n';
    }
  else {
    for(uint32_t j = 0u; j < mCoeffCount; ++j) {
      mCoefficients(j) = 0.0f;
    }
    mRrmsError = std::numeric_limits<double>::max();
  }
}

double PolynomApprox::eval(double const aX) const {
  auto x = normalize(aX);
  double result = mCoefficients(mCoeffCount - 1u) * x;
  for(uint32_t i = mCoeffCount - 2u; i > 0u; --i) {
    result = (result + mCoefficients(i)) * x;
  }
  result += mCoefficients(0);
  return result / ::pow(x, mDegreeMinus);
}
