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
#include "Helpers.h"
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

static void WriteImagePixelsToRGBSpace(const FloatVectorImageType::Pointer image, const std::string& outputFileName);
static void WriteClusteredPixelsInRGBSpace(const FloatVectorImageType::Pointer image, const unsigned int numberOfClusters,
                                           const std::string& outputFileName);

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

  HelpersOutput::WriteVectorImageAsRGB(reader->GetOutput(), outputPrefix + "/Image.mha");

  WriteImagePixelsToRGBSpace(reader->GetOutput(), outputPrefix + "/ImageColors.vtp");

  WriteClusteredPixelsInRGBSpace(reader->GetOutput(), 20, outputPrefix + "/ImageColorsClustered.vtp");

  FloatVectorImageType::Pointer blurred = FloatVectorImageType::New();
  //float blurVariance = 2.0f; // almost no visible blurring
  //float blurVariance = 10.0f; // slight blurring of concrete
  float blurVariance = 30.0f;
  Helpers::AnisotropicBlurAllChannels<FloatVectorImageType>(reader->GetOutput(), blurred, blurVariance);

  HelpersOutput::WriteVectorImageAsRGB(blurred, outputPrefix + "/BlurredImage.mha");

  WriteImagePixelsToRGBSpace(blurred, outputPrefix + "/BlurredImageColors.vtp");

  WriteClusteredPixelsInRGBSpace(blurred, 20, outputPrefix + "/BlurredImageColorsClustered.vtp");

  return EXIT_SUCCESS;
}


void WriteClusteredPixelsInRGBSpace(const FloatVectorImageType::Pointer image, const unsigned int numberOfClusters, const std::string& outputFileName)
{
  std::cout << "WriteClusteredPixelsInRGBSpace()" << std::endl;
  ClusterColorsAdaptive clusterColors;
  clusterColors.SetNumberOfColors(numberOfClusters);
  clusterColors.ConstructFromImage(image);

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkUnsignedCharArray> colorsVTK = vtkSmartPointer<vtkUnsignedCharArray>::New();
  colorsVTK->SetName("Colors");
  colorsVTK->SetNumberOfComponents(3);

  vtkSmartPointer<vtkUnsignedIntArray> ids = vtkSmartPointer<vtkUnsignedIntArray>::New();
  ids->SetName("Ids");
  ids->SetNumberOfComponents(1);

  ColorMeasurementVectorType queryPoint;
  std::vector<ColorMeasurementVectorType> colors = clusterColors.GetColors();
  itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(image, image->GetLargestPossibleRegion());
  while(!imageIterator.IsAtEnd())
    {
    FloatVectorImageType::PixelType pixel = imageIterator.Get();
    queryPoint[0] = pixel[0];
    queryPoint[1] = pixel[1];
    queryPoint[2] = pixel[2];

    ClusterColors::TreeType::InstanceIdentifierVectorType neighbors;
    clusterColors.GetKDTree()->Search( queryPoint, 1u, neighbors );
    points->InsertNextPoint(pixel[0], pixel[1], pixel[2]);
    unsigned char color[3] = {colors[neighbors[0]][0], colors[neighbors[0]][1], colors[neighbors[0]][2]};
    colorsVTK->InsertNextTupleValue(color);
    ids->InsertNextValue(neighbors[0]);
    ++imageIterator;
    }

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(points);
  polyData->GetPointData()->SetScalars(colorsVTK);
  polyData->GetPointData()->AddArray(ids);

  vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
  glyphFilter->SetInputConnection(polyData->GetProducerPort());
  glyphFilter->Update();

  vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  writer->SetInputConnection(glyphFilter->GetOutputPort());
  writer->SetFileName(outputFileName.c_str());
  writer->Write();

}

void WriteImagePixelsToRGBSpace(const FloatVectorImageType::Pointer image, const std::string& outputFileName)
{
  std::cout << "WriteImagePixelsToRGBSpace()" << std::endl;
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  colors->SetName("Colors");
  colors->SetNumberOfComponents(3);

  itk::ImageRegionConstIterator<FloatVectorImageType> imageIterator(image, image->GetLargestPossibleRegion());
  while(!imageIterator.IsAtEnd())
    {
    FloatVectorImageType::PixelType pixel = imageIterator.Get();
    points->InsertNextPoint(pixel[0], pixel[1], pixel[2]);
    unsigned char color[3] = {pixel[0], pixel[1], pixel[2]};
    colors->InsertNextTupleValue(color);
    ++imageIterator;
    }

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(points);
  polyData->GetPointData()->SetScalars(colors);

  vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
  glyphFilter->SetInputConnection(polyData->GetProducerPort());
  glyphFilter->Update();

  vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  writer->SetInputConnection(glyphFilter->GetOutputPort());
  writer->SetFileName(outputFileName.c_str());
  writer->Write();

}
