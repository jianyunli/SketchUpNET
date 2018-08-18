/*

SketchUpNET - a C++ Wrapper for the Trimble(R) SketchUp(R) C API
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

#include <SketchUpAPI/slapi.h>
#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/initialize.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/model.h>
#include <SketchUpAPI/model/entities.h>
#include <SketchUpAPI/model/face.h>
#include <SketchUpAPI/model/edge.h>
#include <SketchUpAPI/model/vertex.h>
#include <msclr/marshal.h>
#include <vector>
#include "Utilities.h"
#include "Surface.h"
#include "Edge.h"
#include "Curve.h"
#include "Layer.h"
#include "Group.h"
#include "Instance.h"
#include "Component.h"

#pragma once

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;

namespace SketchUpNET
{
	public enum class SKPVersion
	{
		V2013,
		V2014,
		V2015,
		V2016,
		V2017,
		V2018
	};

	/// <summary>
	/// SKP
	/// </summary>
	public ref class SketchUp
	{


	public:
		/// <summary>
		/// Surfaces
		/// </summary>
		System::Collections::Generic::List<Surface^>^ Surfaces;

		/// <summary>
		/// Layers
		/// </summary>
		System::Collections::Generic::List<Layer^>^ Layers;

		/// <summary>
		/// Groups
		/// </summary>
		System::Collections::Generic::List<Group^>^ Groups;

		/// <summary>
		/// Component Definitions
		/// </summary>
		System::Collections::Generic::Dictionary<String^, Component^>^ Components;

		/// <summary>
		/// Material Definitions
		/// </summary>
		System::Collections::Generic::Dictionary<String^, Material^>^ Materials;

		/// <summary>
		/// Component Instances
		/// </summary>
		System::Collections::Generic::List<Instance^>^ Instances;

		/// <summary>
		/// Curves (Arcs)
		/// </summary>
		System::Collections::Generic::List<Curve^>^ Curves; 

		/// <summary>
		/// Edges (Lines)
		/// </summary>
		System::Collections::Generic::List<Edge^>^ Edges;

		/// <summary>
		/// Load SKP Model without Meshes
		/// </summary>
		bool LoadModel(System::String^ filename)
		{
			return LoadModel(filename, false);
		}

		/// <summary>
		/// Load SKP Model
		/// </summary>
		bool LoadModel(System::String^ filename, bool includeMeshes)
		{
			msclr::interop::marshal_context oMarshalContext;

			const char* path = oMarshalContext.marshal_as<const char*>(filename);

			SUInitialize();


			SUModelRef model = SU_INVALID;

			SUResult res = SUModelCreateFromFile(&model, path);

			

			if (res != SU_ERROR_NONE)
				return false;


			Layers = gcnew System::Collections::Generic::List<Layer^>();
			Groups = gcnew System::Collections::Generic::List<Group^>();
			Components = gcnew System::Collections::Generic::Dictionary<String^,Component^>();
			Materials = gcnew System::Collections::Generic::Dictionary<String^, Material^>();

			SUEntitiesRef entities = SU_INVALID;
			SUModelGetEntities(model, &entities);

			//Get All Materials
			size_t matCount = 0;
			SUModelGetNumMaterials(model, &matCount);

			if (matCount > 0) {
				std::vector<SUMaterialRef> materials(matCount);
				SUModelGetMaterials(model, matCount, &materials[0], &matCount);

				for (size_t i = 0; i < matCount; i++) {
					Material^ mat = Material::FromSU(materials[i]);
					if (!Materials->ContainsKey(mat->Name))
						Materials->Add(mat->Name, mat);
				}
			}
			
			//Get All Layers
			size_t layerCount = 0;
			SUModelGetNumLayers(model, &layerCount);

			if (layerCount > 0) {
				std::vector<SULayerRef> layers(layerCount);
				SUModelGetLayers(model, layerCount, &layers[0], &layerCount);

				for (size_t i = 0; i < layerCount; i++) {
					Layer^ layer = Layer::FromSU(layers[i]);
					Layers->Add(layer);
				}
			}

			//Get All Groups	
			size_t groupCount = 0;
			SUEntitiesGetNumGroups(entities, &groupCount);

			if (groupCount > 0) {
				std::vector<SUGroupRef> groups(groupCount);
				SUEntitiesGetGroups(entities, groupCount, &groups[0], &groupCount);

				for (size_t i = 0; i < groupCount; i++) {
					Group^ group = Group::FromSU(groups[i], includeMeshes, Materials);
					Groups->Add(group);
				}

			}


			// Get all Components
			size_t compCount = 0;
			SUModelGetNumComponentDefinitions(model, &compCount);

			if (compCount > 0) {
				std::vector<SUComponentDefinitionRef> comps(compCount);
				SUModelGetComponentDefinitions(model, compCount, &comps[0], &compCount);

				for (size_t i = 0; i < compCount; i++) {
					Component^ component = Component::FromSU(comps[i], includeMeshes, Materials);
					Components->Add(component->Guid, component);
				}
			}

			Surfaces = Surface::GetEntitySurfaces(entities, includeMeshes, Materials);
			Curves = Curve::GetEntityCurves(entities);
			Edges = Edge::GetEntityEdges(entities);
			Instances = Instance::GetEntityInstances(entities);

			for each (Instance^ var in Instances)
			{
				if (Components->ContainsKey(var->ParentID))
				{
					System::Object^ o = Components[var->ParentID];
					var->Parent = o;
				}
			}

			for each (KeyValuePair<String^, Component^>^ cmp in Components)
			{
				FixRefs(cmp->Value);
			}

			for each (Group^ var in Groups)
			{
				FixRefs(var);
			}


			SUModelRelease(&model);
			SUTerminate();
			return true;

		};

		bool SaveAs(System::String^ filename, SKPVersion version, System::String^ newFilename)
		{
			msclr::interop::marshal_context oMarshalContext;
			const char* path = oMarshalContext.marshal_as<const char*>(filename);
			SUInitialize();

			SUModelRef model = SU_INVALID;
			SUResult res = SUModelCreateFromFile(&model, path);

			if (res != SU_ERROR_NONE)
				return false;

			SUModelVersion saveversion = SUModelVersion::SUModelVersion_SU2018;

			if (version == SKPVersion::V2013)
				saveversion = SUModelVersion::SUModelVersion_SU2013;
			else if (version == SKPVersion::V2014)
				saveversion = SUModelVersion::SUModelVersion_SU2014;
			else if (version == SKPVersion::V2015)
				saveversion = SUModelVersion::SUModelVersion_SU2015;
			else if (version == SKPVersion::V2016)
				saveversion = SUModelVersion::SUModelVersion_SU2016;
			else if (version == SKPVersion::V2017)
				saveversion = SUModelVersion::SUModelVersion_SU2017;
			else if (version == SKPVersion::V2018)
				saveversion = SUModelVersion::SUModelVersion_SU2018;

			SUModelSaveToFileWithVersion(model, Utilities::ToString(newFilename), saveversion);

			SUModelRelease(&model);
			SUTerminate();
			return true;
		}

		/// <summary>
		/// Append Data to existing Model
		/// </summary>
		bool AppendToModel(System::String^ filename)
		{
			msclr::interop::marshal_context oMarshalContext;

			const char* path = oMarshalContext.marshal_as<const char*>(filename);

			SUInitialize();


			SUModelRef model = SU_INVALID;

			SUResult res = SUModelCreateFromFile(&model, path);



			if (res != SU_ERROR_NONE)
				return false;


			SUEntitiesRef entities = SU_INVALID;
			SUModelGetEntities(model, &entities);

			SUEntitiesAddFaces(entities, Surfaces->Count, Surface::ListToSU(Surfaces));
			SUEntitiesAddEdges(entities, Edges->Count, Edge::ListToSU(Edges));
			SUEntitiesAddCurves(entities, Curves->Count, Curve::ListToSU(Curves));

			SUModelSaveToFile(model, Utilities::ToString(filename));
			
			SUModelRelease(&model);
			SUTerminate();
			return true;

		};

		/// <summary>
		/// Write to new empty Model
		/// </summary>
		bool WriteNewModel(System::String^ filename)
		{
			SUInitialize();
			SUModelRef model = SU_INVALID;
			SUResult res = SUModelCreate(&model);

			if (res != SU_ERROR_NONE) return false;


			SUEntitiesRef entities = SU_INVALID;
			SUModelGetEntities(model, &entities);

			SUEntitiesAddFaces(entities, Surfaces->Count, Surface::ListToSU(Surfaces));
			SUEntitiesAddEdges(entities, Edges->Count, Edge::ListToSU(Edges));
			SUEntitiesAddCurves(entities, Curves->Count, Curve::ListToSU(Curves));
			
			
			SUModelSaveToFile(model, Utilities::ToString(filename));
			SUModelRelease(&model);
			SUTerminate();

			return true;
		}

		private:


			void FixRefs(Component^ comp)
			{
				for each (Instance^ var in comp->Instances)
				{
					if (Components->ContainsKey(var->ParentID))
					{
						System::Object^ o = Components[var->ParentID];
						var->Parent = o;

						FixRefs(Components[var->ParentID]);
					}
				}
			}

			void FixRefs(Group^ comp)
			{
				for each (Instance^ var in comp->Instances)
				{
					if (Components->ContainsKey(var->ParentID))
					{
						System::Object^ o = Components[var->ParentID];
						var->Parent = o;

						FixRefs(Components[var->ParentID]);
					}
				}
			}


	};


}