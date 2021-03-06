#include "g4rcUniformScattering.hh"

#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4PhysicalConstants.hh"
#include "Randomize.hh"

#include "CLHEP/Random/RandFlat.h"

#include <math.h>

g4rcUniformScattering::g4rcUniformScattering(const G4String& processName)
 : G4VDiscreteProcess(processName, fNotDefined) {

	fVertexZ = 7.5*cm;
	fEcut = 1.*MeV;
	fThetaCentral = 17.5*deg;
	fHasScattered = false;
	
}



g4rcUniformScattering::~g4rcUniformScattering() {
}



G4double g4rcUniformScattering::PostStepGetPhysicalInteractionLength(const G4Track& aTrack, G4double previousStepSuze, G4ForceCondition* condition) {

	*condition = NotForced;

	G4double length = DBL_MAX;

	// Only limit step if track is primary

	if(aTrack.GetTrackID()==1) {

		G4double z = aTrack.GetPosition().z();
	
		if(z < fVertexZ) {
			length = fVertexZ - z;
		}
	}

	return length;
}



G4VParticleChange* g4rcUniformScattering::PostStepDoIt(const G4Track& aTrack, const G4Step& aStep) {

	aParticleChange.Initialize(aTrack);

	if(!fHasScattered) {

		G4double Mp = 0.938272*GeV;

		G4double Epre = aTrack.GetTotalEnergy();
		G4double Ekin = aTrack.GetKineticEnergy();

		// Choose theta
		G4double fThetaMin = fThetaCentral - 5.*deg;
		G4double fThetaMax = fThetaCentral + 5.*deg;	
		G4double fCosThMin = cos(fThetaMax);
		G4double fCosThMax = cos(fThetaMin);
		G4double theta = acos(CLHEP::RandFlat::shoot(fCosThMin, fCosThMax));

		// Set upper Q2 limit and sample
		G4double fQ2Min = 0.;
		G4double fQ2Max = (2.*Mp*Epre)/(1.+(Mp/(Epre*(1.-cos(theta)))));
		G4double Q2_born = CLHEP::RandFlat::shoot(fQ2Min, fQ2Max);

		// Choose phi;
		G4double fPhiMin = -10.*deg;
		G4double fPhiMax = +10.*deg;
		G4double phi = CLHEP::RandFlat::shoot(fPhiMin, fPhiMax);

		G4double internal_loss1;
		G4double E0, Ef, nu_born;

		nu_born = -1.;

		int n_tries = 0;
		bool good_kin = true;

		while((nu_born < 0.) || (nu_born < (Q2_born/(2.*Mp)))) {
			if(n_tries >= 10) {
				good_kin = false;
				break;
			}
			internal_loss1 = RadiateInternal(Q2_born, Ekin);
			E0 = Epre - internal_loss1;
			Ef = Q2_born/(2.*E0*(1. - cos(theta)));
			nu_born = E0 - Ef;
			n_tries++;
		}

		Ekin = Ekin - internal_loss1 - nu_born;
		G4double internal_loss2 = RadiateInternal(Q2_born, Ekin);

		G4double Epost = Ef - internal_loss2;

		G4ThreeVector p = G4ThreeVector(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta));

		fEpre = Epre;
		fE0 = E0;
		fEp = Ef;
		fEpost = Epost;
		fTheta = theta;
		fPhi = phi;
		fQ2born = Q2_born;
		
		if(good_kin) {
			aParticleChange.ProposeEnergy(Epost);
			aParticleChange.ProposeMomentumDirection(p);
		} else {
			aParticleChange.ProposeTrackStatus(fStopAndKill);
		}

		fHasScattered = true;
	}
	return &aParticleChange;			

}


G4double g4rcUniformScattering::RadiateInternal(G4double Q2, G4double E) {

	G4double Euler = 0.5772157;
	G4double alpha = 1./137.;

	// Equivalent radiator thickness from Mo and Tsai
	G4double bt = (alpha/pi)*(log(Q2/(electron_mass_c2*electron_mass_c2)) - 1.);

	G4double eps = fEcut/E;

	G4double prob, prob_sample, sample, Eloss, value;

	value = 1.;
	
	prob = 1. - pow(eps, bt) - (bt/(bt+1.))*(1. - pow(eps, bt+1.)) + (3./4.)*(bt/(bt+2.))*(1. - pow(eps, bt+2.));
	prob/=(1. - bt*Euler + ((bt*bt)/12.)*(6.*Euler*Euler + pi*pi));

	prob_sample = G4UniformRand();

	G4double env, ref;

	Eloss = 0.;

	if (prob_sample <= prob) {
		do {
			sample = G4UniformRand();
			Eloss = fEcut*pow(E/fEcut, sample);
			env = 1./Eloss;
			value = (1./Eloss)*pow(Eloss/E, bt)*(1. - (Eloss/E) + (3./4.)*pow(Eloss/E, 2.));

			sample = G4UniformRand();
			ref = value/env;
		} while (sample > ref);
	}

	return Eloss;

}



void g4rcUniformScattering::SetVertexZ(G4double z) {
	fVertexZ = z;
}



void g4rcUniformScattering::SetCentralScatteringAngle(G4double th) {
	fThetaCentral = th;
}

