#include <iostream>
#include "DNest4.h"
#include "Data.h"
#include "RVmodel.h"

using namespace std;
using namespace DNest4;

/* priors */
//  data-dependent priors should be defined in the RVmodel() 
//  constructor and use Data::get_instance() 
#include "default_priors.h"


const bool obs_after_HARPS_fibers = false;
const bool GP = false;
const bool hyperpriors = false;
const bool trend = false;

// options for the model
// 
RVmodel::RVmodel()
    :planets(5, 1, false, RVConditionalPrior())
    ,mu(Data::get_instance().N())
    ,C(Data::get_instance().N(), Data::get_instance().N())
{
    Cprior = new Uniform(-10, 10); // m/s
    Jprior = new ModifiedLogUniform(1.0, 2000.); // m/s

    Pprior = new LogUniform(0.2, 1E2); // days
    Kprior = new ModifiedLogUniform(1.0, 2E3); // m/s

    eprior = new Uniform(0., 1.);
    phiprior = new Uniform(0.0, 2*M_PI);
    wprior = new Uniform(0.0, 2*M_PI);

    save_setup();
}


int main(int argc, char** argv)
{
    /* set the RV data file */
    char* datafile = "51Peg.rv";

    // the third (optional) argument, 
    // tells kima not to skip any line in the header of the file
    Data::get_instance().load(datafile, "ms", 0);
    
    // set the sampler and run it!
    Sampler<RVmodel> sampler = setup<RVmodel>(argc, argv);
    sampler.run(10);

    return 0;
}