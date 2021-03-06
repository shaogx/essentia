/*
 * Copyright (C) 2006-2016  Music Technology Group - Universitat Pompeu Fabra
 *
 * This file is part of Essentia
 *
 * Essentia is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the Free
 * Software Foundation (FSF), either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the Affero GNU General Public License
 * version 3 along with this program.  If not, see http://www.gnu.org/licenses/
 */

#include "TriangularBands.h"
#include "essentiamath.h"

namespace essentia{
namespace standard{

const char* TriangularBands::name = "TriangularBands";
const char* TriangularBands::description = DOC("This algorithm computes the energy of an input spectrum for an arbitrary number of overlapping Triangular frequency bands. For each band the power-spectrum (mag-squared) is summed.\n"
"\n"
"Parameter \"TriangularBands\" must contain at least 2 frequencies, they all must be positive and must be ordered ascentdantly, otherwise an exception will be thrown. TriangularBands is only defined for spectrum, which size is greater than 1.\n");


void TriangularBands::configure() {
  _bandFrequencies = parameter("frequencyBands").toVectorReal();
  _sampleRate = parameter("sampleRate").toReal();
  if ( _bandFrequencies.size() < 2 ) {
    throw EssentiaException("TriangularBands: the 'frequencyBands' parameter contains only one element (i.e. two elements are required to construct a band)");
  }
  
  for (int i=1; i < int(_bandFrequencies.size()); ++i) {
    if (_bandFrequencies[i] < 0) {
      throw EssentiaException("TriangularBands: the 'frequencyBands' parameter contains a negative value");
    }
    if (_bandFrequencies[i-1] >= _bandFrequencies[i]) {
      throw EssentiaException("TriangularBands: the values in the 'frequencyBands' parameter are not in ascending order or there exists a duplicate value");
    }
  }
  _isLog = parameter("log").toBool();
}


void TriangularBands::compute() {
  const std::vector<Real>& spectrum = _spectrumInput.get();
  std::vector<Real>& bands = _bandsOutput.get();

  if (spectrum.size() <= 1) {
    throw EssentiaException("TriangularBands: the size of the input spectrum is not greater than one");
  }

  Real frequencyScale = (_sampleRate / 2.0) / (spectrum.size() - 1);
  int nBands = int(_bandFrequencies.size() - 2);

  bands.resize(nBands);
  std::fill(bands.begin(), bands.end(), (Real) 0.0);

  for (int i=0; i<nBands; i++) {

    int startBin = int(_bandFrequencies[i] / frequencyScale +.5);
    int midBin = int(_bandFrequencies[i + 1] / frequencyScale +.5 );
    int endBin = int(_bandFrequencies[i + 2] / frequencyScale +.5);
  
	  // finished
    if (startBin >= int(spectrum.size())) break;

    // going to far
    if (endBin > int(spectrum.size())) endBin = spectrum.size();
	
    //Compute normalization factor
    Real norm = 0;
    if (midBin != startBin && midBin != endBin && endBin != startBin) {
      for (int j=startBin; j<=endBin; j++) {
        norm +=  j < midBin ? (j-startBin) / (midBin - startBin) 
                            : 1 - (j-midBin) / (endBin-midBin);
      }
    }

    for (int j=startBin; j <= endBin; j++) {
      Real TriangF;
      if (midBin != startBin && midBin != endBin && endBin != startBin) {
        TriangF = j < midBin ? (j-startBin) / (midBin - startBin) 
                             : 1 - (j-midBin) / (endBin-midBin);
        TriangF /= norm;
	    }
      else if (startBin == endBin) {
        // case of single bin band
        TriangF = 1;
      }
      else {
        // double bin band
		    TriangF = 0.5;
	    }
	
      bands[i] += TriangF * spectrum[j] * spectrum[j]; 
    }
    if (_isLog) bands[i] = log2(1 + bands[i]);
  }
}

} // namespace standard
} // namespace essentia

