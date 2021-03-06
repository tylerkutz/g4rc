#ifndef g4rcDetectorConstruction_h
#define g4rcDetectorConstruction_h 

#include "G4VUserDetectorConstruction.hh"

class g4rcMaterial;

class g4rcDetectorConstruction : public G4VUserDetectorConstruction {
	
public:
	g4rcDetectorConstruction();
	~g4rcDetectorConstruction();
	
	G4VPhysicalVolume* Construct();

	void SetTarget(G4String);

public:
	G4String fHRS;
	G4double fHRSAngle;	

private:
	G4int fTargIndex;

	G4String fTarg;

	g4rcMaterial* fMaterial;

	void GetTargetIndex(G4String);

};

#endif
