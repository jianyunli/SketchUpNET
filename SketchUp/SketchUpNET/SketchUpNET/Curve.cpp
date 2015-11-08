/*

SketchUpNET - a managed C++ Wrapper for the SketchUp C API
Copyright(C) 2015, Autor: Maximilian Thumfart

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <slapi/slapi.h>
#include <slapi/geometry.h>
#include <slapi/initialize.h>
#include <slapi/unicodestring.h>
#include <slapi/model/model.h>
#include <slapi/model/entities.h>
#include <slapi/model/face.h>
#include <slapi/model/edge.h>
#include <slapi/model/vertex.h>
#include <slapi/model/curve.h>
#include <msclr/marshal.h>
#include <vector>
#include "edge.h"

#pragma once

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;

namespace SketchUpNET
{
	public ref class Curve
	{
	public:

		System::Collections::Generic::List<Edge^>^ Edges = gcnew System::Collections::Generic::List<Edge^>();

		Curve(System::Collections::Generic::List<Edge^>^ edges)
		{
			this->Edges = edges;
		};

		Curve(){};
	internal:
		static Curve^ FromSU(SUCurveRef curve)
		{
			System::Collections::Generic::List<Edge^>^ edgelist = gcnew System::Collections::Generic::List<Edge^>();

			size_t edgecount = 0;
			SUCurveGetNumEdges(curve, &edgecount);
			if (edgecount > 0)
			{
				std::vector<SUEdgeRef> edges(edgecount);
				SUCurveGetEdges(curve, edgecount, &edges[0], &edgecount);

				for (size_t j = 0; j < edgecount; j++)
				{
					edgelist->Add(Edge::FromSU(edges[j]));
				}
			}

		
			Curve^ v = gcnew Curve(edgelist);

			return v;
		};




	};




}