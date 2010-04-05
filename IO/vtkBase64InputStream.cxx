/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkBase64InputStream.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkBase64InputStream.h"
#include "vtkObjectFactory.h"
#include "vtkBase64Utilities.h"

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkBase64InputStream, "1.4");
vtkStandardNewMacro(vtkBase64InputStream);

//----------------------------------------------------------------------------
vtkBase64InputStream::vtkBase64InputStream()
{
  this->BufferLength = 0;
}

//----------------------------------------------------------------------------
vtkBase64InputStream::~vtkBase64InputStream()
{
}

//----------------------------------------------------------------------------
void vtkBase64InputStream::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
inline int vtkBase64InputStream::DecodeTriplet(unsigned char& c0,
                                               unsigned char& c1,
                                               unsigned char& c2)
{
  // Read the 4 bytes encoding this triplet from the stream.

  unsigned char in[4];
  this->Stream->read(reinterpret_cast<char*>(in), 4);
  if(this->Stream->gcount() < 4) { return 0; }

  return vtkBase64Utilities::DecodeTriplet(in[0], in[1], in[2], in[3],
                                           &c0, &c1, &c2);
}

//----------------------------------------------------------------------------
void vtkBase64InputStream::StartReading()
{
  this->Superclass::StartReading();
  this->BufferLength = 0;
}

//----------------------------------------------------------------------------
void vtkBase64InputStream::EndReading()
{
}

//----------------------------------------------------------------------------
int vtkBase64InputStream::Seek(unsigned long offset)
{
  unsigned long triplet = offset/3;
  int skipLength = offset%3;
  
  // Seek to the beginning of the encoded triplet containing the
  // offset.
  if(!this->Stream->seekg(this->StreamStartPosition+(triplet*4)))
    {
    return 0;
    }
  
  // Decode the first triplet if it is paritally skipped.
  if(skipLength == 0)
    {
    this->BufferLength = 0;
    }
  else if(skipLength == 1)
    {
    unsigned char c;
    this->BufferLength =
      this->DecodeTriplet(c, this->Buffer[0], this->Buffer[1]) - 1;
    }
  else
    {
    unsigned char c[2];
    this->BufferLength =
      this->DecodeTriplet(c[0], c[1], this->Buffer[0]) - 2;
    }
  
  // A DecodeTriplet call may have failed to read the stream.
  // If so, the current buffer length will be negative.
  return ((this->BufferLength >= 0) ? 1:0);
}

//----------------------------------------------------------------------------
unsigned long vtkBase64InputStream::Read(unsigned char* data,
                                         unsigned long length)
{
  unsigned char* out = data;
  unsigned char* end = data + length;
  
  // If the previous read ended before filling buffer, don't read
  // more.
  if(this->BufferLength < 0) { return 0; }
  
  // Use leftover characters from a previous decode.
  if((out != end) && (this->BufferLength == 2))
    {
    *out++ = this->Buffer[0];
    this->Buffer[0] = this->Buffer[1];
    this->BufferLength = 1;
    }
  if((out != end) && (this->BufferLength == 1))
    {
    *out++ = this->Buffer[0];
    this->BufferLength = 0;
    }
  
  // Decode all complete triplets.
  while((end - out) >= 3)
    {
    int len = this->DecodeTriplet(out[0], out[1], out[2]);
    out += len;
    if(len < 3)
      {
      this->BufferLength = len-3;
      return (out-data);
      }
    }
  
  // Decode the last triplet and save leftover characters in the buffer.
  if((end - out) == 2)
    {
    int len = this->DecodeTriplet(out[0], out[1], this->Buffer[0]);
    this->BufferLength = len-2;
    if(len > 2) { out += 2; }
    else { out += len; }
    }
  else if((end - out) == 1)
    {
    int len = this->DecodeTriplet(out[0], this->Buffer[0], this->Buffer[1]);
    this->BufferLength = len-1;
    if(len > 1) { out += 1; }
    else { out += len; }
    }
  
  return (out-data);  
}