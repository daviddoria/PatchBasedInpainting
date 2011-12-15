/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

// Custom
#include "HelpersOutput.h"
#include "Mask.h"
#include "Types.h"
#include "ClusterColorsUniform.h"
#include "ClusterColorsAdaptive.h"

// ITK
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

// VTK
#include <vtkCellData.h>
#include <vtkSmartPointer.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkXMLPolyDataWriter.h>

int main(int argc, char *argv[])
{
  std::string inputFileName = argv[1];
  std::string outputPrefix = argv[2];

  std::cout << "Input: " << inputFileName << std::endl;
  std::cout << "Output prefix: " << outputPrefix << std::endl;

  typedef itk::ImageFileReader<FloatVectorImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(inputFileName);
  reader->Update();

  ClusterColorsAdaptive clusterColors;
  clusterColors.SetNumberOfColors(100);
  clusterColors.ConstructFromImage(reader->GetOutput());

  std::vector<ColorMeasurementVectorType> colorMeasurements = clusterColors.GetColors();

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  colors->SetName("Colors");
  colors->SetNumberOfComponents(3);

  for(unsigned int colorId = 0; colorId < colorMeasurements.size(); ++colorId)
    {
    double point[3];
    unsigned char color[3];
    for(unsigned int component = 0; component < 3; component++)
      {
      point[component] = colorMeasurements[colorId][component];
      color[component] = colorMeasurements[colorId][component];
      }
    std::cout << "ColorId " << colorId << " : " << point[0] << " " << point[1] << " " << point[2] << std::endl;
    points->InsertNextPoint(point);
    colors->InsertNextTupleValue(color);
    }
  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(points);
  polyData->GetPointData()->SetScalars(colors);

  vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
  glyphFilter->SetInputConnection(polyData->GetProducerPort());
  glyphFilter->Update();

  vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  writer->SetInputConnection(glyphFilter->GetOutputPort());
  writer->SetFileName((outputPrefix + ".vtp").c_str());
  writer->Write();

  return EXIT_SUCCESS;
}
