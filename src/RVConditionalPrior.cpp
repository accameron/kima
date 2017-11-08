#include "RVConditionalPrior.h"
#include "DNest4.h"
#include "Utils.h"
#include <cmath>
#include <typeinfo>

#include "options.h"


using namespace std;
using namespace DNest4;


#if hyperpriors
    // Cauchy prior centered on log(365 days), scale=1
    // for log(muP), with muP in days
    // -- truncated to (~-15.1, ~26.9) --
    TruncatedCauchy log_muP_prior(log(365), 1., log(365)-21, log(365)+21);

    // Uniform prior between 0.1 and 3
    // for wP
    Uniform wP_prior(0.1, 3.);
    
    // Cauchy prior centered on log(1), scale=1
    // for log(muK), with muK in m/s
    // -- truncated to (-21, 21) --
    // NOTE: we actually sample on muK itself, just the prior is for log(muK). 
    // Confusing, I know...
    TruncatedCauchy log_muK_prior(0., 1., 0.-21, 0.+21);

    Laplace Pprior;
    Exponential Kprior;

#else
    //extern Jeffreys Pprior;
    extern ContinuousDistribution *Pprior;
    extern ContinuousDistribution *Kprior;
    // Jeffreys Pprior(1.0, 1E7); // days
    //ModifiedJeffreys Kprior(1.0, 1E4); // m/s
    //Uniform Kprior(0., 20.); // m/s
#endif

    extern ContinuousDistribution *eprior;
    extern ContinuousDistribution *phiprior;
    extern ContinuousDistribution *wprior;
    //TruncatedRayleigh eprior(0.2, 0.0, 1.0);
    // Uniform eprior(0., 1.);
    //TruncatedNormal eprior(0, 0.3, 0., 1.);
    // Uniform phiprior(0.0, 2*M_PI);
    // Uniform wprior(0.0, 2*M_PI);


RVConditionalPrior::RVConditionalPrior()
{
    //cout << "type of Pprior:" << typeid(Pprior).name() << endl;
}


void RVConditionalPrior::from_prior(RNG& rng)
{
    //cout << "called RVConditionalPrior::from_prior !!!" << endl;
    #if hyperpriors
        center = log_muP_prior.rvs(rng);
        width = wP_prior.rvs(rng);
        mu = exp(log_muK_prior.rvs(rng));
    #endif
}

double RVConditionalPrior::perturb_hyperparameters(RNG& rng)
{
    //cout << "called RVConditionalPrior::perturb_hyperparameters !!!" << endl;
    double logH = 0.;

    #if hyperpriors
        int which = rng.rand_int(3);

        if(which == 0)
        {
            logH -= log_muP_prior.log_pdf(center);
            center = log_muP_prior.rvs(rng);
            logH += log_muP_prior.log_pdf(center);
        }
        else if(which == 1)
        {
            width = wP_prior.rvs(rng);
        }
        else
        {
            mu = log(mu);

            logH -= log_muK_prior.log_pdf(mu);
            mu = log_muK_prior.rvs(rng);
            logH += log_muK_prior.log_pdf(mu);      

            mu = exp(mu);
        }
    #endif

    return logH;
}

// vec[0] = period
// vec[1] = amplitude
// vec[2] = phase
// vec[3] = ecc
// vec[4] = viewing angle

double RVConditionalPrior::log_pdf(const std::vector<double>& vec) const
{
    //cout << "type of Pprior:" << typeid(Pprior).name() << endl;
    //cout << "called RVConditionalPrior::log_pdf !!!" << endl;
    #if hyperpriors
        if(vec[2] < 0. || vec[2] > 2.*M_PI ||
           vec[3] < 0. || vec[3] >= 1.0 ||
           vec[4] < 0. || vec[4] > 2.*M_PI)
             return -1E300;

        Pprior = Laplace(center, width);
        Kprior = Exponential(mu);

    #else
        if(vec[0] < 1. || vec[0] > 1E4 ||
           vec[1] < 0. ||
           vec[2] < 0. || vec[2] > 2.*M_PI ||
           vec[3] < 0. || vec[3] >= 1.0 ||
           vec[4] < 0. || vec[4] > 2.*M_PI)
             return -1E300;

    #endif

    return Pprior->log_pdf(vec[0]) + 
           Kprior->log_pdf(vec[1]) + 
           phiprior->log_pdf(vec[2]) + 
           eprior->log_pdf(vec[3]) + 
           wprior->log_pdf(vec[4]);
}

void RVConditionalPrior::from_uniform(std::vector<double>& vec, int id) const
{
    #if hyperpriors
        Pprior = Laplace(center, width);
        Kprior = Exponential(mu);
    #endif
    vec[0] = Pprior->cdf_inverse(vec[0]);
    vec[1] = Kprior->cdf_inverse(vec[1]);
    vec[2] = phiprior->cdf_inverse(vec[2]); //2.*M_PI*vec[2];
    vec[3] = eprior->cdf_inverse(vec[3]);
    vec[4] = wprior->cdf_inverse(vec[4]); //2.*M_PI*vec[4];
}

void RVConditionalPrior::to_uniform(std::vector<double>& vec, int id) const
{
    #if hyperpriors
        Pprior = Laplace(center, width);
        Kprior = Exponential(mu);
    #endif
    vec[0] = Pprior->cdf(vec[0]);
    vec[1] = Kprior->cdf(vec[1]);
    vec[2] = phiprior->cdf(vec[2]); //vec[2]/(2.*M_PI);
    vec[3] = eprior->cdf(vec[3]);
    vec[4] = wprior->cdf(vec[4]); //vec[4]/(2.*M_PI);
}

void RVConditionalPrior::print(std::ostream& out) const
{
    #if hyperpriors
        out<<center<<' '<<width<<' '<<mu<<' ';
    #endif
}


void RVConditionalPrior::print0(std::ostream& out) const
{
    out<<0.<<' '<<0.<<' '<<0.<<' ';
}