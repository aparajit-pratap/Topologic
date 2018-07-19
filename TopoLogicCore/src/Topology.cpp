#include <Topology.h>
#include <Cluster.h>
#include <CellComplex.h>
#include <Cell.h>
#include <Shell.h>
#include <Face.h>
#include <Wire.h>
#include <Edge.h>
#include <Vertex.h>
#include <Context.h>
#include <ShapeFix_ShapeTolerance.hxx>

#include <TopoDS_Vertex.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Face.hxx>
#include <BOPAlgo_CellsBuilder.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <TopoDS.hxx>
//#include <TNaming_Builder.hxx>
//#include <TDF_ChildIterator.hxx>
//#include <TDataStd_Integer.hxx>
#include <ShapeAnalysis_ShapeContents.hxx>

#include <BOPAlgo_Builder.hxx>
#include <BOPAlgo_CellsBuilder.hxx>
#include <BOPAlgo_Splitter.hxx>
#include <BOPCol_ListOfShape.hxx>
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <ShapeAnalysis_ShapeContents.hxx>
#include <ShapeBuild_ReShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Builder.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_FrozenShape.hxx>
#include <TopoDS_UnCompatibleShapes.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TNaming_NamedShape.hxx>
#include <TDataStd_Integer.hxx>
#include <BRepCheck_Wire.hxx>
#include <BOPAlgo_MakerVolume.hxx>
#include <TopTools_DataMapOfShapeInteger.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepFill_Filling.hxx>

#include <BRepAdaptor_Surface.hxx>

#include <ShapeFix_Shape.hxx>

#include <array>
#include <algorithm>
#include <queue>
#include <set>

namespace TopologicCore
{
	void AddOcctListShapeToAnotherList(const BOPCol_ListOfShape& rkAList, BOPCol_ListOfShape& rAnotherList)
	{
		for (BOPCol_ListIteratorOfListOfShape kIterator(rkAList);
			kIterator.More();
			kIterator.Next())
		{
			rAnotherList.Append(kIterator.Value());
		}
	}

	std::string Topology::GetTypeAsString() const
	{
		switch (GetType())
		{
		case TOPOLOGY_CLUSTER:
			return std::string("Cluster");
		case TOPOLOGY_CELLCOMPLEX:
			return std::string("CellComplex");
		case TOPOLOGY_CELL:
			return std::string("Cell");
		case TOPOLOGY_SHELL:
			return std::string("Shell");
		case TOPOLOGY_FACE:
			return std::string("Face");
		case TOPOLOGY_WIRE:
			return std::string("Wire");
		case TOPOLOGY_EDGE:
			return std::string("Edge");
		default: // case TOPOLOGY_VERTEX:
			return std::string("Vertex");
		}
	}

	/*void Topology::AddChildLabel(Topology::Ptr& pTopology, const TopologyRelationshipType kRelationshipType)
	{
		TDF_Label occtChildLabel = TDF_TagSource::NewChild(GetOcctLabel());
		TNaming_Builder cellAttributeBuilder(occtChildLabel);
		cellAttributeBuilder.Generated(pTopology->GetOcctShape());
		TDataStd_Integer::Set(occtChildLabel, kRelationshipType);
		pTopology->SetOcctLabel(occtChildLabel);
	}*/

	//TDF_Label& Topology::GetOcctLabel()
	//{
	//	//assert(m_occtLabel.IsNull() && "OCAF label is null.");
	//	if (m_occtLabel.IsNull())
	//	{
	//		//throw std::exception("OCAF label is null.");
	//	}

	//	return m_occtLabel;
	//}

	//const TDF_Label & Topology::GetOcctLabel() const
	//{
	//	//assert(m_occtLabel.IsNull() && "OCAF label is null.");
	//	if (m_occtLabel.IsNull())
	//	{
	//		//throw std::exception("OCAF label is null.");
	//	}

	//	return m_occtLabel;
	//}

	//void Topology::SetOcctLabel(const TDF_Label & rkOcctLabel)
	//{
	//	// Only register the label if it is not under the root
	//	if (rkOcctLabel.IsNull() || !LabelManager::GetInstance().HasLabel(rkOcctLabel))
	//	{
	//		RegisterLabel();
	//	}
	//	else
	//	{
	//		m_occtLabel = rkOcctLabel;
	//	}
	//}

	Topology::Ptr Topology::ClosestLowestSubshapeTo(const Topology::Ptr& kpTopology) const
	{
		//TopTools_DataMapOfShapeInteger occtShapeToDistanceMap;
		TopoDS_Shape occtClosestSubshape;
		double minDistance = std::numeric_limits<double>::max();
		const TopoDS_Shape& kOcctThisShape = GetOcctShape();
		const TopoDS_Shape& kOcctQueryShape = kpTopology->GetOcctShape();
		TopAbs_ShapeEnum occtShapeTypes[4] = { TopAbs_VERTEX, TopAbs_EDGE, TopAbs_FACE, TopAbs_SOLID };
		//for (int i = 0; i < 3; ++i)
		for (int i = 0; i < 4; ++i)
		{
			TopAbs_ShapeEnum occtShapeType = occtShapeTypes[i];
			TopTools_MapOfShape occtCells;
			for (TopExp_Explorer occtExplorer(kOcctThisShape, occtShapeType); occtExplorer.More(); occtExplorer.Next())
			{
				const TopoDS_Shape& rkCurrentChildShape = occtExplorer.Current();
				BRepExtrema_DistShapeShape occtDistanceCalculation(rkCurrentChildShape, kOcctQueryShape);
				bool isDone = occtDistanceCalculation.Perform();
				if (isDone)
				{
					double distance = occtDistanceCalculation.Value();
					if (distance < minDistance)
					{
						minDistance = distance;
						occtClosestSubshape = rkCurrentChildShape;
					}
					else if (minDistance <= distance && 
						     distance <= minDistance + Precision::Confusion() && 
							 rkCurrentChildShape.ShapeType() > occtClosestSubshape.ShapeType()) // larger value = lower dimension
					{
						TopAbs_ShapeEnum closestShapeType = occtClosestSubshape.ShapeType();
						TopAbs_ShapeEnum currentShapeType = rkCurrentChildShape.ShapeType();

						minDistance = distance;
						occtClosestSubshape = rkCurrentChildShape;
					}
				}
			}
		}

		if (occtClosestSubshape.IsNull())
		{
			return nullptr;
		}

		return Topology::ByOcctShape(occtClosestSubshape);
	}

	/*TDF_Attribute * Topology::FindAttribute(const Standard_GUID & kID) const
	{
		return LabelManager::FindAttribute(GetOcctLabel(), kID);
	}*/

	double Topology::Distance(const Topology::Ptr& kpTopology) const
	{
		BRepExtrema_DistShapeShape occtDistance(GetOcctShape(), kpTopology->GetOcctShape());
		return occtDistance.Value();
	}

	Topology::Topology(const int kDimensionality)
		: m_dimensionality(kDimensionality)
		/*, m_isInGlobalCluster(false)
		, m_occtLabel()*/
	{

	}

	Topology::Ptr Topology::ByOcctShape(const TopoDS_Shape& rkOcctShape)
	{
		if (rkOcctShape.TShape().IsNull()) 
		{
			return nullptr;
		}
		TopAbs_ShapeEnum occtShapeType = rkOcctShape.ShapeType();
		switch (occtShapeType)
		{
		case TopAbs_COMPOUND: return std::make_shared<Cluster>(TopoDS::Compound(rkOcctShape));// , rkOcctLabel);
		case TopAbs_COMPSOLID: return std::make_shared<CellComplex>(TopoDS::CompSolid(rkOcctShape));//, rkOcctLabel);
		case TopAbs_SOLID: return std::make_shared<Cell>(TopoDS::Solid(rkOcctShape));
		case TopAbs_SHELL: return std::make_shared<Shell>(TopoDS::Shell(rkOcctShape));
		case TopAbs_FACE: return std::make_shared<Face>(TopoDS::Face(rkOcctShape));
		case TopAbs_WIRE: return std::make_shared<Wire>(TopoDS::Wire(rkOcctShape));
		case TopAbs_EDGE: return std::make_shared<Edge>(TopoDS::Edge(rkOcctShape));
		case TopAbs_VERTEX: return std::make_shared<Vertex>(TopoDS::Vertex(rkOcctShape));
		default:
			throw std::exception("Topology::ByOcctShape: unknown topology.");
		}
	}

	TopoDS_CompSolid Topology::MakeCompSolid(const TopoDS_Shape& rkOcctShape)
	{
		TopoDS_CompSolid occtCompSolid;
		BRep_Builder occtBuilder;
		occtBuilder.MakeCompSolid(occtCompSolid);

		TopTools_MapOfShape occtCells;
		for (TopExp_Explorer occtExplorer(rkOcctShape, TopAbs_SOLID); occtExplorer.More(); occtExplorer.Next())
		{
			const TopoDS_Shape& occtCurrent = occtExplorer.Current();
			if (!occtCells.Contains(occtCurrent))
			{
				occtCells.Add(occtCurrent);
				try {
					occtBuilder.Add(occtCompSolid, occtCurrent);
				}
				catch (TopoDS_FrozenShape&)
				{
					throw std::exception("The cell is not free and cannot be modified.");
				}
				catch (TopoDS_UnCompatibleShapes&)
				{
					throw std::exception("The cell and face are not compatible.");
				}
			}
		}

		return occtCompSolid;
	}

	Topology::~Topology()
	{
		// Clear ingredients information
		/*for (std::list<Topology*>::const_iterator kTopologyIterator = m_ingredientTo.begin();
			kTopologyIterator != m_ingredientTo.end();
			kTopologyIterator++)
		{
			Topology* pTopology = *kTopologyIterator;
			pTopology->m_ingredients.remove(this);
		}*/
		/*m_ingredientTo.clear();
		m_ingredients.clear();*/
		m_contents.clear();
		m_contexts.clear();

		/*for (AttributeMapIterator attributeMapIterator = m_attributeMap.begin();
			attributeMapIterator != m_attributeMap.end();
			attributeMapIterator++)
		{
			delete attributeMapIterator->second;
		}*/
		m_attributeMap.clear();
	}

	Topology::Ptr Topology::ByGeometry(Handle(Geom_Geometry) pGeometry)
	{
		return nullptr;
	}

	Topology::Ptr Topology::ByContext(const std::shared_ptr<Context>& kpContext)
	{
		return nullptr;
	}

	/*Topology::Ptr Topology::ByVertexIndex(const std::list<std::array<double, 3>>& rkVertexCoordinates, const std::list<std::list<int>>& rkVertexIndices)
	{
		return nullptr;
	}*/

	void Topology::ByVertexIndex(const std::vector<Vertex::Ptr>& rkVertices, const std::list<std::list<int>>& rkVertexIndices, std::list<Topology::Ptr>& rTopologies)
	{
		if (rkVertices.empty() || rkVertexIndices.empty())
		{
			return;
		}

		for (const std::list<int>& rkVertex1DIndices : rkVertexIndices)
		{
			if (rkVertex1DIndices.empty())
				continue;

			std::list<TopoDS_Vertex> occtVertices;
			for (int vertexIndex : rkVertex1DIndices)
			{
				occtVertices.push_back(rkVertices[vertexIndex]->GetOcctVertex());
			}

			if (occtVertices.size() > 2)
			{
				//// Always assume it tries to build a face, so add the first index again.
				//occtVertices.push_back(rkVertices[*rkVertex1DIndices.begin()]->GetOcctVertex());

				// Create edges
				TopTools_ListOfShape occtEdges;
				std::list<TopoDS_Vertex>::const_iterator kEndVertexIterator = occtVertices.end();
				kEndVertexIterator--;

				for (std::list<TopoDS_Vertex>::const_iterator kVertexIterator = occtVertices.begin();
					kVertexIterator != kEndVertexIterator;
					kVertexIterator++)
				{
					std::list<TopoDS_Vertex>::const_iterator kNextVertexIterator = kVertexIterator;
					kNextVertexIterator++;

					BRepBuilderAPI_MakeEdge occtMakeEdge(TopoDS::Vertex(*kVertexIterator), TopoDS::Vertex(*kNextVertexIterator));
					occtEdges.Append(occtMakeEdge);
				}
				// No need to connect the first and last vertices

				// Creating a face
				BRepBuilderAPI_MakeWire occtMakeWire;
				occtMakeWire.Add(occtEdges);
				if (occtMakeWire.Error() == BRepBuilderAPI_WireDone)
				{
					const TopoDS_Wire& rkOcctWire = occtMakeWire.Wire();

					if (BRepCheck_Wire(rkOcctWire).Closed() == BRepCheck_NoError)
					{
						BRepBuilderAPI_MakeFace occtMakeFace(rkOcctWire);
						if (occtMakeFace.Error() == BRepBuilderAPI_FaceDone)
						{
							rTopologies.push_back(Topology::ByOcctShape(occtMakeFace.Face()));
						}
						else
						{
							// Add the closed wire
							rTopologies.push_back(Topology::ByOcctShape(rkOcctWire));
						}
					}
					else
					{
						// Add the opem wire
						rTopologies.push_back(Topology::ByOcctShape(rkOcctWire));
					}
				}
				else
				{
					// Add the edges
					for (TopTools_ListIteratorOfListOfShape occtEdgeIterator(occtEdges);
						occtEdgeIterator.More();
						occtEdgeIterator.Next())
					{
						rTopologies.push_back(Topology::ByOcctShape(occtEdgeIterator.Value()));
					}
				}
			}
			else if (occtVertices.size() == 2)
			{
				// Try creating an edge
				BRepBuilderAPI_MakeEdge occtMakeEdge(TopoDS::Vertex(occtVertices.front()),
					TopoDS::Vertex(occtVertices.back()));
				rTopologies.push_back(Topology::ByOcctShape(occtMakeEdge));
			}
			else if (occtVertices.size() == 1)
			{
				// Insert the vertices
				rTopologies.push_back(Topology::ByOcctShape(occtVertices.front()));
			}
		}
	}

	void Topology::AddContent(const Topology::Ptr& rkTopology)
	{
		//LabelManager::GetInstance().AddContent(rkTopology->GetOcctLabel(), GetOcctLabel());
	}

	void Topology::RemoveContent(const Topology::Ptr& rkTopology)
	{
		m_contents.remove(rkTopology);
	}

	void Topology::AddContext(const std::shared_ptr<Context>& rkContext)
	{
		if (std::find(m_contexts.begin(), m_contexts.end(), rkContext) == m_contexts.end())
		{
			m_contexts.push_back(rkContext);
		}
	}

	void Topology::RemoveContext(const std::shared_ptr<Context>& rkContext)
	{
		m_contexts.remove(rkContext);
	}

	void Topology::SharedTopologies(const Topology::Ptr& kpTopology, std::list<Topology::Ptr>& rkSharedTopologies) const
	{
	}

	void Topology::PathsTo(const Topology::Ptr& kpTopology, const Topology::Ptr& kpParentTopology, const int kMaxLevels, const int kMaxPaths, std::list<std::list<std::shared_ptr<TopologicalQuery>>>& rkPaths) const
	{
		// Use BFS.
		// https://en.wikipedia.org/wiki/Breadth-first_search
		//std::queue<Topology const *> openSetTopology;
		//std::set<Topology const *> closedSetTopology;
		//topologyToVisit.push_back(this);
		//std::list<Topology const *> visitedTopologies;
		//while (!topologyToVisit.empty())
		//{
		//	Topology const * kpCurrentTopology = topologyToVisit.front();
		//	topologyToVisit.pop_front();
		//	visitedTopologies.push_back(kpCurrentTopology);

		//	// Get neighbours
		//	std::list<Topology const *> neighbours; // up and down

		//	DownwardNavigation(neighbours);
		//	UpwardNavigation(kpParentTopology, neighbours);

		//	for(Topology const * kpNeighbour : neighbours)
		//	{
		//		if()
		//	}
		//}
	}

	void Topology::Contents(std::list<Topology::Ptr>& rContents) const
	{
		//LabelManager::Contents(GetOcctLabel(), rContents);
	}

	void Topology::ContentsV2(const bool kAllLevels, std::list<Topology::Ptr>& rContents) const
	{
		//const TDF_Label& rkOcctLabel = GetOcctLabel();
		//if (rkOcctLabel.IsNull())
		//{
		//	return;
		//}

		//// Get the list of attribute shapes in the child labels in all levels, therefore no need to iterate hierarchically.
		//for (TDF_ChildIterator occtLabelIterator(rkOcctLabel, kAllLevels); occtLabelIterator.More(); occtLabelIterator.Next())
		//{
		//	TDF_Label childLabel = occtLabelIterator.Value();
		//	// Only care about those labels with aperture or other non-constituent relationships.
		//	Handle(TNaming_NamedShape) occtApertureAttribute;
		//	Handle(TDataStd_Integer) occtRelationshipType;
		//	bool result1 = childLabel.FindAttribute(TNaming_NamedShape::GetID(), occtApertureAttribute);
		//	bool result2 = childLabel.FindAttribute(TDataStd_Integer::GetID(), occtRelationshipType);
		//	int result3 = occtRelationshipType->Get();
		//	if (result1 &&
		//		result2 &&
		//		occtRelationshipType->Get() != REL_CONSTITUENT)
		//	{
		//		Topology::Ptr pTopology = Topology::ByOcctShape(occtApertureAttribute->Get());
		//		pTopology->SetOcctLabel(childLabel);
		//		rContents.push_back(pTopology);
		//	}
		//}
	}

	bool Topology::SaveToBrep(const std::string & rkPath) const
	{
		return BRepTools::Write(GetOcctShape(), rkPath.c_str());;
	}

	Topology::Ptr Topology::LoadFromBrep(const std::string & rkPath)
	{
		TopoDS_Shape occtShape;
		BRep_Builder occtBRepBuilder;
		bool returnValue = BRepTools::Read(occtShape, rkPath.c_str(), occtBRepBuilder);

		return Topology::ByOcctShape(occtShape);
	}

	std::string Topology::Analyze(const TopoDS_Shape& rkShape, const int kLevel)
	{
		BOPCol_ListOfShape occtImmediateMembers;
		ImmediateMembers(rkShape, occtImmediateMembers);

		std::array<std::string, 8> occtShapeNameSingular;
		occtShapeNameSingular[0] = "a cluster";
		occtShapeNameSingular[1] = "a cellComplex";
		occtShapeNameSingular[2] = "a cell";
		occtShapeNameSingular[3] = "a shell";
		occtShapeNameSingular[4] = "a face";
		occtShapeNameSingular[5] = "a wire";
		occtShapeNameSingular[6] = "an edge";
		occtShapeNameSingular[7] = "a vertex";

		std::array<std::string, 8> occtShapeNamePlural;
		occtShapeNamePlural[0] = "clusters";
		occtShapeNamePlural[1] = "cellComplexes";
		occtShapeNamePlural[2] = "cells";
		occtShapeNamePlural[3] = "shells";
		occtShapeNamePlural[4] = "faces";
		occtShapeNamePlural[5] = "wires";
		occtShapeNamePlural[6] = "edges";
		occtShapeNamePlural[7] = "vertices";

		TopAbs_ShapeEnum occtShapeType = rkShape.ShapeType();
		std::stringstream ssCurrentIndent;
		for (int i = 0; i < kLevel; ++i)
		{
			ssCurrentIndent << "  ";
		}
		std::string currentIndent = ssCurrentIndent.str();
		int numberOfSubentities[8] = { 0,0,0,0,0,0,0,0 };
		for (BOPCol_ListIteratorOfListOfShape kMemberIterator(occtImmediateMembers);
			kMemberIterator.More();
			kMemberIterator.Next())
		{
			const TopoDS_Shape& rkMemberTopology = kMemberIterator.Value();
			TopAbs_ShapeEnum occtShapeMemberType = rkMemberTopology.ShapeType();
			numberOfSubentities[occtShapeMemberType]++;
		}

		std::stringstream ssCurrentResult;

		// For the topmost level only, print the overall subentities result
		if (kLevel == 0)
		{
			ShapeAnalysis_ShapeContents occtShapeAnalysis;
			occtShapeAnalysis.Perform(rkShape);

			// No method is provided in ShapeAnalysis_ShapeContents to compute the number of CompSolids.
			// Do this manually.
			int numberCompSolids = 0;
			BOPCol_ListOfShape occtCompSolids;
			for (TopExp_Explorer occtExplorer(rkShape, TopAbs_COMPSOLID); occtExplorer.More(); occtExplorer.Next())
			{
				const TopoDS_Shape& rkOcctCurrent = occtExplorer.Current();
				if (!occtCompSolids.Contains(rkOcctCurrent))
				{
					occtCompSolids.Append(rkOcctCurrent);
					numberCompSolids++;
				}
			}
			ssCurrentResult <<
				"OVERALL ANALYSIS" << std::endl <<
				"================" << std::endl <<
				"The shape is " << occtShapeNameSingular[occtShapeType] << "." << std::endl <<
				"Number of cell complexes = " << numberCompSolids << std::endl <<
				"Number of cells = " << occtShapeAnalysis.NbSharedSolids() << std::endl <<
				"Number of shells = " << occtShapeAnalysis.NbSharedShells() << std::endl <<
				"Number of faces = " << occtShapeAnalysis.NbSharedFaces() << std::endl <<
				"Number of wires = " << occtShapeAnalysis.NbSharedWires() << std::endl <<
				"Number of edges = " << occtShapeAnalysis.NbSharedEdges() << std::endl <<
				"Number of vertices = " << occtShapeAnalysis.NbSharedVertices() << std::endl <<
				std::endl << std::endl <<
				"INDIVIDUAL ANALYSIS" << std::endl <<
				"================" << std::endl;
		}

		ssCurrentResult << currentIndent << "The shape is " << occtShapeNameSingular[occtShapeType] << "." << std::endl;

		for (int i = occtShapeType + 1; i < 8; ++i)
		{
			if(numberOfSubentities[i] > 0)
			{
				ssCurrentResult << currentIndent << "Number of " << occtShapeNamePlural[i] << " = " << numberOfSubentities[i] << std::endl;
			}
		}
		ssCurrentResult << currentIndent << "================" << std::endl;

		for (BOPCol_ListIteratorOfListOfShape kMemberIterator(occtImmediateMembers);
			kMemberIterator.More();
			kMemberIterator.Next())
		{
			const TopoDS_Shape& rkMemberTopology = kMemberIterator.Value();
			std::string strMemberAnalyze = Analyze(rkMemberTopology, kLevel + 1);
			ssCurrentResult << strMemberAnalyze;
		}

		return ssCurrentResult.str();
	}

	std::string Topology::Analyze()
	{
		return Analyze(GetOcctShape(), 0);
	}

	void Topology::BooleanImages(
		const Topology::Ptr& kpOtherTopology,
		std::list<Topology::Ptr>& kArgumentImagesInArguments,
		std::list<Topology::Ptr>& kArgumentImagesInTools,
		std::list<Topology::Ptr>& kToolsImagesInArguments,
		std::list<Topology::Ptr>& kToolsImagesInTools)
	{
		BOPCol_ListOfShape occtArgumentImagesInArguments;
		BOPCol_ListOfShape occtArgumentImagesInTools;
		BOPCol_ListOfShape occtToolsImagesInArguments;
		BOPCol_ListOfShape occtToolsImagesInTools;
		BOPCol_ListOfShape occtExclusivelyArgumentImages;
		BOPCol_ListOfShape occtExclusivelyToolImages;
		BOPCol_ListOfShape occtSharedImages;
		BOPCol_ListOfShape occtExclusivelyArgumentImageFaces;
		BOPCol_ListOfShape occtExclusivelyToolImageFaces;
		BOPCol_ListOfShape occtSharedImageFaces;
;
		BOPAlgo_CellsBuilder occtCellsBuilder;
		BooleanImages(
			kpOtherTopology,
			occtCellsBuilder,
			occtExclusivelyArgumentImages,
			occtExclusivelyToolImages,
			occtSharedImages,
			occtExclusivelyArgumentImageFaces,
			occtExclusivelyToolImageFaces,
			occtSharedImageFaces,
			occtArgumentImagesInArguments, 
			occtArgumentImagesInTools, 
			occtToolsImagesInArguments, 
			occtToolsImagesInTools);

		for (BOPCol_ListIteratorOfListOfShape kImageIterator(occtArgumentImagesInArguments);
			kImageIterator.More();
			kImageIterator.Next())
		{
			kArgumentImagesInArguments.push_back(Topology::ByOcctShape(kImageIterator.Value()));
		}
		for (BOPCol_ListIteratorOfListOfShape kImageIterator(occtArgumentImagesInTools);
			kImageIterator.More();
			kImageIterator.Next())
		{
			kArgumentImagesInTools.push_back(Topology::ByOcctShape(kImageIterator.Value()));
		}
		for (BOPCol_ListIteratorOfListOfShape kImageIterator(occtToolsImagesInArguments);
			kImageIterator.More();
			kImageIterator.Next())
		{
			kToolsImagesInArguments.push_back(Topology::ByOcctShape(kImageIterator.Value()));
		}
		for (BOPCol_ListIteratorOfListOfShape kImageIterator(occtToolsImagesInTools);
			kImageIterator.More();
			kImageIterator.Next())
		{
			kToolsImagesInTools.push_back(Topology::ByOcctShape(kImageIterator.Value()));
		}
	}

	void Topology::BooleanImages(
		const Topology::Ptr& kpOtherTopology,
		BOPAlgo_CellsBuilder& rOcctCellsBuilder,
		BOPCol_ListOfShape& rOcctExclusivelyArgumentImages,
		BOPCol_ListOfShape& rOcctExclusivelyToolImages,
		BOPCol_ListOfShape& rOcctSharedImages,
		BOPCol_ListOfShape& rOcctExclusivelyArgumentImageFaces,
		BOPCol_ListOfShape& rOcctExclusivelyToolImageFaces,
		BOPCol_ListOfShape& rOcctSharedImageFaces,
		BOPCol_ListOfShape& rOcctArgumentImagesInArguments, 
		BOPCol_ListOfShape& rOcctArgumentImagesInTools, 
		BOPCol_ListOfShape& rOcctToolsImagesInArguments, 
		BOPCol_ListOfShape& rOcctToolsImagesInTools)
	{
		BOPCol_ListOfShape occtCellsBuildersArguments;
		BOPCol_ListOfShape occtCellsBuildersOperandsA;
		if (GetType() == TOPOLOGY_CELLCOMPLEX)
		{
			CellComplex* pCellComplex = TopologicalQuery::Downcast<CellComplex>(this);
			std::list<Cell::Ptr> cells;
			pCellComplex->Cells(cells);
			for (const Cell::Ptr& kCell : cells)
			{
				Handle(ShapeFix_Shape) sfs = new ShapeFix_Shape();
				sfs->Init(kCell->GetOcctShape());
				sfs->SetPrecision(Precision::Confusion());
				sfs->Perform();
				occtCellsBuildersOperandsA.Append(sfs->Shape());
				occtCellsBuildersArguments.Append(sfs->Shape());
			}
		}
		else
		{
			Handle(ShapeFix_Shape) sfs = new ShapeFix_Shape();
			sfs->Init(GetOcctShape());
			sfs->SetPrecision(Precision::Confusion());
			sfs->Perform();
			occtCellsBuildersOperandsA.Append(sfs->Shape());
			occtCellsBuildersArguments.Append(sfs->Shape());
		}

		BOPCol_ListOfShape occtCellsBuildersOperandsB;
		if (kpOtherTopology->GetType() == TOPOLOGY_CELLCOMPLEX)
		{
			CellComplex::Ptr pCellComplex = TopologicalQuery::Downcast<CellComplex>(kpOtherTopology);
			std::list<Cell::Ptr> cells;
			pCellComplex->Cells(cells);
			for (const Cell::Ptr& kCell : cells)
			{
				Handle(ShapeFix_Shape) sfs = new ShapeFix_Shape();
				sfs->Init(kCell->GetOcctShape());
				sfs->SetPrecision(Precision::Confusion());
				sfs->Perform();
				occtCellsBuildersOperandsB.Append(sfs->Shape());
				occtCellsBuildersArguments.Append(sfs->Shape());
			}
		}
		else
		{
			Handle(ShapeFix_Shape) sfs = new ShapeFix_Shape();
			sfs->Init(kpOtherTopology->GetOcctShape());
			sfs->SetPrecision(Precision::Confusion());
			sfs->Perform();
			occtCellsBuildersOperandsB.Append(sfs->Shape());
			occtCellsBuildersArguments.Append(sfs->Shape());
		}

		rOcctCellsBuilder.SetArguments(occtCellsBuildersArguments);

		// Split the arguments and tools
		rOcctCellsBuilder.Perform();

		if (rOcctCellsBuilder.HasErrors())
		{
			std::ostringstream errorStream;
			rOcctCellsBuilder.DumpErrors(errorStream);
			throw std::exception(errorStream.str().c_str());
		}
		const TopoDS_Shape& rMergeResult = rOcctCellsBuilder.Shape();

		const BOPCol_DataMapOfShapeListOfShape& rkImages = rOcctCellsBuilder.Images();
		int size = rkImages.Size();

		const TopoDS_Shape& rkParts = rOcctCellsBuilder.GetAllParts();
		Topology::Ptr pTopologyParts = Topology::ByOcctShape(rkParts);
		std::string strParts = pTopologyParts->Analyze();

		// 3. Classify the images: exclusively from argument, exclusively from tools, or shared.
		for (BOPCol_ListIteratorOfListOfShape kArgumentIterator(occtCellsBuildersOperandsA);
			kArgumentIterator.More();
			kArgumentIterator.Next())
		{
			const BOPCol_ListOfShape& rkArgumentImages = rkImages.Find(kArgumentIterator.Value());

			for (BOPCol_ListIteratorOfListOfShape kArgumentImageIterator(rkArgumentImages);
				kArgumentImageIterator.More();
				kArgumentImageIterator.Next())
			{
				const TopoDS_Shape& rkArgumentImage = kArgumentImageIterator.Value();

				// Is this in tool too?
				bool isArgumentImageInTools = false;
				for (BOPCol_ListIteratorOfListOfShape kToolIterator(occtCellsBuildersOperandsB);
					kArgumentImageIterator.More() && !isArgumentImageInTools;
					kArgumentImageIterator.Next())
				{
					const BOPCol_ListOfShape& rkToolImages = rkImages.Find(kToolIterator.Value());

					for (BOPCol_ListIteratorOfListOfShape kToolImageIterator(rkToolImages);
						kToolImageIterator.More() && !isArgumentImageInTools;
						kToolImageIterator.Next())
					{
						if (rkArgumentImage.IsSame(kToolImageIterator.Value()))
						{
							isArgumentImageInTools = true;
						}
					}
				}

				if (isArgumentImageInTools)
				{
					rOcctSharedImages.Append(rkArgumentImage);
				}
				else
				{
					rOcctExclusivelyArgumentImages.Append(rkArgumentImage);
				}
			}
		}

		for (BOPCol_ListIteratorOfListOfShape kToolIterator(occtCellsBuildersOperandsB);
			kToolIterator.More();
			kToolIterator.Next())
		{
			const BOPCol_ListOfShape& rkToolImages = rkImages.Find(kToolIterator.Value());

			for (BOPCol_ListIteratorOfListOfShape kToolImageIterator(rkToolImages);
				kToolImageIterator.More();
				kToolImageIterator.Next())
			{
				const TopoDS_Shape& rkToolImage = kToolImageIterator.Value();

				// Is this in tool too?
				bool isToolInArguments = false;
				for (BOPCol_ListIteratorOfListOfShape kArgumentIterator(occtCellsBuildersOperandsA);
					kArgumentIterator.More() && !isToolInArguments;
					kArgumentIterator.Next())
				{
					const BOPCol_ListOfShape& rkArgumentImages = rkImages.Find(kArgumentIterator.Value());

					for (BOPCol_ListIteratorOfListOfShape kArgumentImageIterator(rkArgumentImages);
						kArgumentImageIterator.More() && !isToolInArguments;
						kArgumentImageIterator.Next())
					{
						if (rkToolImage.IsSame(kArgumentImageIterator.Value()))
						{
							isToolInArguments = true;
						}
					}
				}

				if (!isToolInArguments)
				{
					rOcctExclusivelyToolImages.Append(rkToolImage);
				}

				// The shared images have been taken care of; no need to check them.
			}
		}

		// 4. Get the faces
		for (BOPCol_ListIteratorOfListOfShape kArgumentImageIterator(rOcctExclusivelyArgumentImages);
			kArgumentImageIterator.More();
			kArgumentImageIterator.Next())
		{
			for (TopExp_Explorer occtArgumentImageExplorer(kArgumentImageIterator.Value(), TopAbs_FACE); 
				 occtArgumentImageExplorer.More(); 
				 occtArgumentImageExplorer.Next())
			{
				const TopoDS_Shape& rkOcctCurrent = occtArgumentImageExplorer.Current();
				if (!rOcctExclusivelyArgumentImageFaces.Contains(rkOcctCurrent))
				{
					rOcctExclusivelyArgumentImageFaces.Append(rkOcctCurrent);
				}
			}
		}

		for (BOPCol_ListIteratorOfListOfShape kToolImageIterator(rOcctExclusivelyToolImages);
			kToolImageIterator.More();
			kToolImageIterator.Next())
		{
			for (TopExp_Explorer occtToolImageExplorer(kToolImageIterator.Value(), TopAbs_FACE); 
				 occtToolImageExplorer.More(); 
				 occtToolImageExplorer.Next())
			{
				const TopoDS_Shape& rkOcctCurrent = occtToolImageExplorer.Current();
				if (!rOcctExclusivelyToolImageFaces.Contains(rkOcctCurrent))
				{
					rOcctExclusivelyToolImageFaces.Append(rkOcctCurrent);
				}
			}
		}

		for (BOPCol_ListIteratorOfListOfShape kSharedImageIterator(rOcctSharedImages);
			kSharedImageIterator.More();
			kSharedImageIterator.Next())
		{
			for (TopExp_Explorer occtSharedImageExplorer(kSharedImageIterator.Value(), TopAbs_FACE); 
				 occtSharedImageExplorer.More(); 
				 occtSharedImageExplorer.Next())
			{
				const TopoDS_Shape& rkOcctCurrent = occtSharedImageExplorer.Current();
				if (!rOcctSharedImageFaces.Contains(rkOcctCurrent))
				{
					rOcctSharedImageFaces.Append(rkOcctCurrent);
				}
			}
		}

		// 5. Identify the argument faces in a tool by doing IsSame Comparison
		for (BOPCol_ListIteratorOfListOfShape kArgumentImageFaceIterator(rOcctExclusivelyArgumentImageFaces);
			kArgumentImageFaceIterator.More();
			kArgumentImageFaceIterator.Next())
		{
			bool isInSharedImage = false;
			const TopoDS_Shape& rkArgumentImageFace = kArgumentImageFaceIterator.Value();
			for (BOPCol_ListIteratorOfListOfShape kSharedImageFaceIterator(rOcctSharedImageFaces);
				kSharedImageFaceIterator.More() && !isInSharedImage;
				kSharedImageFaceIterator.Next())
			{
				if (rkArgumentImageFace.IsSame(kSharedImageFaceIterator.Value()))
				{
					isInSharedImage = true;
				}
			}

			if (isInSharedImage)
			{
				// part of tools in the arguments
				rOcctToolsImagesInArguments.Append(rkArgumentImageFace);
			}
			else
			{
				rOcctArgumentImagesInArguments.Append(rkArgumentImageFace);
			}
		}

		// 6. Identify the tool faces in an image by doing IsSame Comparison
		for (BOPCol_ListIteratorOfListOfShape kToolImageFaceIterator(rOcctExclusivelyToolImageFaces);
			kToolImageFaceIterator.More();
			kToolImageFaceIterator.Next())
		{
			bool isInSharedImage = false;
			const TopoDS_Shape& rkToolImageFace = kToolImageFaceIterator.Value();
			for (BOPCol_ListIteratorOfListOfShape kSharedImageFaceIterator(rOcctSharedImageFaces);
				kSharedImageFaceIterator.More() && !isInSharedImage;
				kSharedImageFaceIterator.Next())
			{
				if (rkToolImageFace.IsSame(kSharedImageFaceIterator.Value()))
				{
					isInSharedImage = true;
				}
			}

			if (isInSharedImage)
			{
				// part of arguments in the tools
				rOcctArgumentImagesInTools.Append(rkToolImageFace);
			}
			else
			{
				rOcctToolsImagesInTools.Append(rkToolImageFace);
			}
		}

		// 7. Identify faces inside the shared images that are inside the arguments or tools.
		for (BOPCol_ListIteratorOfListOfShape kSharedImageFaceIterator(rOcctSharedImageFaces);
			kSharedImageFaceIterator.More();
			kSharedImageFaceIterator.Next())
		{
			bool isInTools = false;
			const TopoDS_Shape& rkSharedImageFace = kSharedImageFaceIterator.Value();
			for (BOPCol_ListIteratorOfListOfShape kToolImageFaceIterator(rOcctExclusivelyToolImageFaces);
				kToolImageFaceIterator.More() && !isInTools;
				kToolImageFaceIterator.Next())
			{
				if (rkSharedImageFace.IsSame(kToolImageFaceIterator.Value()))
				{
					isInTools = true;
					rOcctArgumentImagesInTools.Append(rkSharedImageFace);
				}
			}

			// If not in the arguments, then it is in the tools.
			if (!isInTools)
			{
				rOcctToolsImagesInArguments.Append(rkSharedImageFace);
			}
		}
	}

	void Topology::BooleanParts(
		const Topology::Ptr& kpOtherTopology,
		std::list<Topology::Ptr>& rSpaceBetween_A_A_and_B_A,
		std::list<Topology::Ptr>& rSpaceBetween_B_A_and_A_B,
		std::list<Topology::Ptr>& rSpaceBetween_A_B_and_B_B)
	{
		BOPCol_ListOfShape occtCellsBuildersArguments;
		BOPCol_ListOfShape occtCellsBuildersOperandsA;
		if (GetType() == TOPOLOGY_CELLCOMPLEX)
		{
			CellComplex* pCellComplex = TopologicalQuery::Downcast<CellComplex>(this);
			std::list<Cell::Ptr> cells;
			pCellComplex->Cells(cells);
			for (const Cell::Ptr& kpCell : cells)
			{
				Handle(ShapeFix_Shape) sfs = new ShapeFix_Shape();
				sfs->Init(kpCell->GetOcctShape());
				sfs->SetPrecision(Precision::Confusion());
				sfs->Perform();
				occtCellsBuildersOperandsA.Append(sfs->Shape());
				occtCellsBuildersArguments.Append(sfs->Shape());
			}
		}
		else
		{
			Handle(ShapeFix_Shape) sfs = new ShapeFix_Shape();
			sfs->Init(GetOcctShape());
			sfs->SetPrecision(Precision::Confusion());
			sfs->Perform();
			occtCellsBuildersOperandsA.Append(sfs->Shape());
			occtCellsBuildersArguments.Append(sfs->Shape());
		}

		BOPCol_ListOfShape occtCellsBuildersOperandsB;
		if (kpOtherTopology->GetType() == TOPOLOGY_CELLCOMPLEX)
		{
			CellComplex::Ptr pCellComplex = TopologicalQuery::Downcast<CellComplex>(kpOtherTopology);
			std::list<Cell::Ptr> cells;
			pCellComplex->Cells(cells);
			for (const Cell::Ptr& kpCell : cells)
			{
				Handle(ShapeFix_Shape) sfs = new ShapeFix_Shape();
				sfs->Init(kpCell->GetOcctShape());
				sfs->SetPrecision(Precision::Confusion());
				sfs->Perform();
				occtCellsBuildersOperandsB.Append(sfs->Shape());
				occtCellsBuildersArguments.Append(sfs->Shape());
			}
		}
		else
		{
			Handle(ShapeFix_Shape) sfs = new ShapeFix_Shape();
			sfs->Init(kpOtherTopology->GetOcctShape());
			sfs->SetPrecision(Precision::Confusion());
			sfs->Perform();
			occtCellsBuildersOperandsB.Append(sfs->Shape());
			occtCellsBuildersArguments.Append(sfs->Shape());
		}

		BOPAlgo_CellsBuilder occtCellsBuilder;
		occtCellsBuilder.SetArguments(occtCellsBuildersArguments);

		// Split the arguments and tools
		occtCellsBuilder.Perform();

		if (occtCellsBuilder.HasErrors())
		{
			std::ostringstream errorStream;
			occtCellsBuilder.DumpErrors(errorStream);
			throw std::exception(errorStream.str().c_str());
		}
		const TopoDS_Shape& rMergeResult = occtCellsBuilder.Shape();

		const BOPCol_DataMapOfShapeListOfShape& rkImages = occtCellsBuilder.Images();


		BOPCol_ListOfShape occtListToTake;
		BOPCol_ListOfShape occtListToAvoid;

		// A_A and B_A
		for (BOPCol_ListIteratorOfListOfShape kOcctShapeIterator(occtCellsBuildersOperandsA);
			kOcctShapeIterator.More();
			kOcctShapeIterator.Next())
		{
			occtCellsBuilder.RemoveAllFromResult();
			occtListToTake.Clear();
			occtListToAvoid.Clear();
			occtListToTake.Append(kOcctShapeIterator.Value());

			for (BOPCol_ListIteratorOfListOfShape kOcctShapeIteratorB(occtCellsBuildersOperandsB);
				kOcctShapeIteratorB.More();
				kOcctShapeIteratorB.Next())
			{
				occtListToAvoid.Append(kOcctShapeIteratorB.Value());
			}
			occtCellsBuilder.AddToResult(occtListToTake, occtListToAvoid);
			occtCellsBuilder.MakeContainers();
			const TopoDS_Shape& occtBooleanResult = occtCellsBuilder.Shape();
			Topology::Ptr pTopology = Topology::ByOcctShape(occtBooleanResult);
			rSpaceBetween_A_A_and_B_A.push_back(pTopology);
		}

		// B_A and A_B
		for (BOPCol_ListIteratorOfListOfShape kOcctShapeIterator(occtCellsBuildersOperandsA);
			kOcctShapeIterator.More();
			kOcctShapeIterator.Next())
		{
			for (BOPCol_ListIteratorOfListOfShape kOcctShapeIteratorB(occtCellsBuildersOperandsB);
				kOcctShapeIteratorB.More();
				kOcctShapeIteratorB.Next())
			{
				occtCellsBuilder.RemoveAllFromResult();
				occtListToTake.Clear();
				occtListToAvoid.Clear();
				occtListToTake.Append(kOcctShapeIterator.Value());
				occtListToTake.Append(kOcctShapeIteratorB.Value());
				occtCellsBuilder.AddToResult(occtListToTake, occtListToAvoid);
				occtCellsBuilder.MakeContainers();
				const TopoDS_Shape& occtBooleanResult = occtCellsBuilder.Shape();
				Topology::Ptr pTopology = Topology::ByOcctShape(occtBooleanResult);
				rSpaceBetween_B_A_and_A_B.push_back(pTopology);
			}
		}

		// A_B and B_B
		for (BOPCol_ListIteratorOfListOfShape kOcctShapeIteratorB(occtCellsBuildersOperandsB);
			kOcctShapeIteratorB.More();
			kOcctShapeIteratorB.Next())
		{
			occtCellsBuilder.RemoveAllFromResult();
			occtListToTake.Clear();
			occtListToAvoid.Clear();
			occtListToTake.Append(kOcctShapeIteratorB.Value());

			for (BOPCol_ListIteratorOfListOfShape kOcctShapeIterator(occtCellsBuildersOperandsA);
				kOcctShapeIterator.More();
				kOcctShapeIterator.Next())
			{
				occtListToAvoid.Append(kOcctShapeIterator.Value());
			}
			occtCellsBuilder.AddToResult(occtListToTake, occtListToAvoid);
			occtCellsBuilder.MakeContainers();
			const TopoDS_Shape& occtBooleanResult = occtCellsBuilder.Shape();
			Topology::Ptr pTopology = Topology::ByOcctShape(occtBooleanResult);
			rSpaceBetween_A_B_and_B_B.push_back(pTopology);
		}
	}

	void Topology::BooleanOperation(
		const Topology::Ptr& kpOtherTopology,
		BOPAlgo_CellsBuilder& rOcctCellsBuilder,
		BOPCol_ListOfShape& rOcctCellsBuildersOperandsA,
		BOPCol_ListOfShape& rOcctCellsBuildersOperandsB,
		BOPCol_DataMapOfShapeShape& rOcctMapFaceToFixedFaceA,
		BOPCol_DataMapOfShapeShape& rOcctMapFaceToFixedFaceB)
	{
		AddBooleanOperands(kpOtherTopology, rOcctCellsBuilder, rOcctCellsBuildersOperandsA, rOcctCellsBuildersOperandsB, rOcctMapFaceToFixedFaceA, rOcctMapFaceToFixedFaceB);

		// Split the arguments and tools
		try {
			rOcctCellsBuilder.Perform();
		}
		catch (Standard_Failure& e)
		{
			const char* str = e.GetMessageString();
			std::string stlStr(str);
		}
		catch (std::exception& e)
		{
			const char* str = e.what();
			std::string stlStr(str);
		}


		if (rOcctCellsBuilder.HasErrors())
		{
			std::ostringstream errorStream;
			rOcctCellsBuilder.DumpErrors(errorStream);
			throw std::exception(errorStream.str().c_str());
		}
	}

	Topology::Ptr Topology::GetBooleanResult(
		const Topology::Ptr& kpOtherTopology,
		BOPAlgo_CellsBuilder& rOcctCellsBuilder,
		BOPCol_DataMapOfShapeShape& rOcctMapFaceToFixedFaceA,
		BOPCol_DataMapOfShapeShape& rOcctMapFaceToFixedFaceB)
	{
		rOcctCellsBuilder.MakeContainers();
		BOPCol_ListOfShape occtImmediateMembers;
		ImmediateMembers(rOcctCellsBuilder.Shape(), occtImmediateMembers);
		if (occtImmediateMembers.Size() == 1)
		{
			BOPCol_ListIteratorOfListOfShape occtImmediateMemberIterator(occtImmediateMembers);
			return Topology::ByOcctShape(occtImmediateMemberIterator.Value());
		}

		return Topology::ByOcctShape(rOcctCellsBuilder.Shape());

		/*Topology::Ptr pTopology;
		if (occtImmediateMembers.Size() == 1)
		{
			pTopology = ManageBooleanLabels(kpOtherTopology, rOcctCellsBuilder, rOcctMapFaceToFixedFaceA, rOcctMapFaceToFixedFaceB);
		}else
		{
			pTopology = ManageBooleanLabels(kpOtherTopology, rOcctCellsBuilder, rOcctMapFaceToFixedFaceA, rOcctMapFaceToFixedFaceB);
		}
		
		return pTopology;*/
	}

	//Topology::Ptr Topology::ManageBooleanLabels(
	//	const Topology::Ptr& kpOtherTopology,
	//	BOPAlgo_CellsBuilder& rOcctCellsBuilder,
	//	BOPCol_DataMapOfShapeShape& rOcctMapFaceToFixedFaceA,
	//	BOPCol_DataMapOfShapeShape& rOcctMapFaceToFixedFaceB)
	//{
	//	// Add the output cluster to the root.
	//	const TopoDS_Shape& rkBooleanResult = rOcctCellsBuilder.Shape();
	//	Topology::Ptr pBooleanResult = Topology::ByOcctShape(rkBooleanResult);
	//	GlobalCluster::GetInstance().GetCluster()->AddChildLabel(pBooleanResult, REL_CONSTITUENT);

	//	// For now, only map the faces and their apertures to the only cell complex in the output cluster.
	//	TopTools_MapOfShape occtCompSolids; 
	//	for (TopExp_Explorer occtExplorer(rOcctCellsBuilder.Shape(), TopAbs_COMPSOLID); occtExplorer.More(); occtExplorer.Next())
	//	{
	//		const TopoDS_Shape& occtCurrent = occtExplorer.Current();
	//		if (!occtCompSolids.Contains(occtCurrent))
	//		{
	//			occtCompSolids.Add(occtCurrent);
	//		}
	//	}
	//	if (occtCompSolids.Size() == 0)
	//	{
	//		return pBooleanResult;
	//	}

	//	// Add the cell complex as the constituent child label to the cluster.
	//	const TopoDS_Shape& rkCompSolid = *occtCompSolids.cbegin();
	//	Topology::Ptr pCellComplex = Topology::ByOcctShape(rkCompSolid);
	//	pBooleanResult->AddChildLabel(pCellComplex, REL_CONSTITUENT);
	//	
	//	// Get the input face labels. These are the labels of the original faces which are not fixed yet,
	//	// so we need to map these faces to the fixed ones.
	//	for (TDF_ChildIterator occtLabelIterator(GetOcctLabel()); occtLabelIterator.More(); occtLabelIterator.Next())
	//	{
	//		TDF_Label childLabel = occtLabelIterator.Value();
	//		Handle(TNaming_NamedShape) occtChildShapeAttribute;
	//		Handle(TDataStd_Integer) occtRelationshipType;
	//		bool result1 = childLabel.FindAttribute(TNaming_NamedShape::GetID(), occtChildShapeAttribute);
	//		bool result2 = childLabel.FindAttribute(TDataStd_Integer::GetID(), occtRelationshipType);
	//		int result3 = occtRelationshipType->Get();

	//		TopoDS_Shape occtFixedFace;
	//		try{
	//			occtFixedFace = rOcctMapFaceToFixedFaceA.Find(occtChildShapeAttribute->Get());
	//		}
	//		catch (...)
	//		{

	//		}

	//		if (result1 &&
	//			result2 &&
	//			occtRelationshipType->Get() == REL_CONSTITUENT &&
	//			!occtFixedFace.IsNull())
	//		{
	//			// Add this label to fixed face label to the cell complex
	//			TopTools_ListOfShape occtModifiedFixedFaces = rOcctCellsBuilder.Modified(occtFixedFace);

	//			for(TopTools_ListIteratorOfListOfShape occtModifiedFixedFaceIterator(occtModifiedFixedFaces);
	//				occtModifiedFixedFaceIterator.More();
	//				occtModifiedFixedFaceIterator.Next())
	//			{
	//				const TopoDS_Shape& rkOcctModifiedFixedFace = occtModifiedFixedFaceIterator.Value();
	//				Topology::Ptr pModifiedFixedFace = Topology::ByOcctShape(rkOcctModifiedFixedFace);
	//				pCellComplex->AddChildLabel(pModifiedFixedFace, REL_CONSTITUENT);

	//				// Transfer the apertures from the original face (if any) to the fixed face
	//				for (TDF_ChildIterator occtFaceLabelIterator(childLabel); occtFaceLabelIterator.More(); occtFaceLabelIterator.Next())
	//				{
	//					TDF_Label apertureLabel = occtFaceLabelIterator.Value();
	//					Handle(TNaming_NamedShape) occtApertureShapeAttribute;
	//					Handle(TDataStd_Integer) occtApertureRelationshipType;
	//					bool result1 = apertureLabel.FindAttribute(TNaming_NamedShape::GetID(), occtApertureShapeAttribute);
	//					bool result2 = apertureLabel.FindAttribute(TDataStd_Integer::GetID(), occtApertureRelationshipType);
	//					int result3 = occtApertureRelationshipType->Get();

	//					if (result1 &&
	//						result2 &&
	//						occtApertureRelationshipType->Get() != REL_CONSTITUENT)
	//					{
	//						Topology::Ptr pAperture = Topology::ByOcctShape(occtApertureShapeAttribute->Get());
	//						pModifiedFixedFace->AddChildLabel(pAperture, (TopologyRelationshipType) occtApertureRelationshipType->Get());
	//					}
	//				}
	//			}
	//		}
	//	}

	//	return pBooleanResult;
	//}

	Topology::Ptr Topology::Difference(const Topology::Ptr& kpOtherTopology)
	{
		BOPCol_ListOfShape occtCellsBuildersOperandsA;
		BOPCol_ListOfShape occtCellsBuildersOperandsB;
		BOPAlgo_CellsBuilder occtCellsBuilder;
		BOPCol_DataMapOfShapeShape occtMapFaceToFixedFaceA;
		BOPCol_DataMapOfShapeShape occtMapFaceToFixedFaceB;

		BooleanOperation(kpOtherTopology, occtCellsBuilder, occtCellsBuildersOperandsA, occtCellsBuildersOperandsB, occtMapFaceToFixedFaceA, occtMapFaceToFixedFaceB);

		BOPCol_ListOfShape occtListToTake;
		BOPCol_ListOfShape occtListToAvoid;

		for (BOPCol_ListIteratorOfListOfShape kOcctShapeIteratorA(occtCellsBuildersOperandsA);
			kOcctShapeIteratorA.More();
			kOcctShapeIteratorA.Next())
		{
			occtListToTake.Clear();
			occtListToAvoid.Clear();
			occtListToTake.Append(kOcctShapeIteratorA.Value());
			for (BOPCol_ListIteratorOfListOfShape kOcctShapeIteratorB(occtCellsBuildersOperandsB);
				kOcctShapeIteratorB.More();
				kOcctShapeIteratorB.Next())
			{
				occtListToAvoid.Append(kOcctShapeIteratorB.Value());
			}
			occtCellsBuilder.AddToResult(occtListToTake, occtListToAvoid);
		}

		return GetBooleanResult(
			kpOtherTopology,
			occtCellsBuilder,
			occtMapFaceToFixedFaceA,
			occtMapFaceToFixedFaceB);
	}

	Topology::Ptr Topology::Impose(const Topology::Ptr& kpOtherTopology)
	{
		BOPCol_ListOfShape occtCellsBuildersOperandsA;
		BOPCol_ListOfShape occtCellsBuildersOperandsB;
		BOPAlgo_CellsBuilder occtCellsBuilder;
		BOPCol_DataMapOfShapeShape occtMapFaceToFixedFaceA;
		BOPCol_DataMapOfShapeShape occtMapFaceToFixedFaceB;

		BooleanOperation(kpOtherTopology, occtCellsBuilder, occtCellsBuildersOperandsA, occtCellsBuildersOperandsB, occtMapFaceToFixedFaceA, occtMapFaceToFixedFaceB);

		BOPCol_ListOfShape occtListToTake;
		BOPCol_ListOfShape occtListToAvoid;

		// Get part of A not in B
		for (BOPCol_ListIteratorOfListOfShape kOcctShapeIteratorA(occtCellsBuildersOperandsA);
			kOcctShapeIteratorA.More();
			kOcctShapeIteratorA.Next())
		{
			occtListToTake.Clear();
			occtListToAvoid.Clear();
			occtListToTake.Append(kOcctShapeIteratorA.Value());
			for (BOPCol_ListIteratorOfListOfShape kOcctShapeIteratorB(occtCellsBuildersOperandsB);
				kOcctShapeIteratorB.More();
				kOcctShapeIteratorB.Next())
			{
				occtListToAvoid.Append(kOcctShapeIteratorB.Value());
			}
			occtCellsBuilder.AddToResult(occtListToTake, occtListToAvoid);
		}

		// Add B
		int i = 1;
		for (BOPCol_ListIteratorOfListOfShape kOcctShapeIteratorB(occtCellsBuildersOperandsB);
			kOcctShapeIteratorB.More();
			kOcctShapeIteratorB.Next())
		{
			occtListToTake.Clear();
			occtListToAvoid.Clear();
			occtListToTake.Append(kOcctShapeIteratorB.Value());
			occtCellsBuilder.AddToResult(occtListToTake, occtListToAvoid, i++, true);
		}

		return GetBooleanResult(
			kpOtherTopology,
			occtCellsBuilder,
			occtMapFaceToFixedFaceA,
			occtMapFaceToFixedFaceB);
	}

	Topology::Ptr Topology::Imprint(const Topology::Ptr& kpOtherTopology)
	{
		BOPCol_ListOfShape occtCellsBuildersOperandsA;
		BOPCol_ListOfShape occtCellsBuildersOperandsB;
		BOPAlgo_CellsBuilder occtCellsBuilder;
		BOPCol_DataMapOfShapeShape occtMapFaceToFixedFaceA;
		BOPCol_DataMapOfShapeShape occtMapFaceToFixedFaceB;

		BooleanOperation(kpOtherTopology, occtCellsBuilder, occtCellsBuildersOperandsA, occtCellsBuildersOperandsB, occtMapFaceToFixedFaceA, occtMapFaceToFixedFaceB);

		BOPCol_ListOfShape occtListToTake;
		BOPCol_ListOfShape occtListToAvoid;
		for (BOPCol_ListIteratorOfListOfShape kOcctShapeIteratorA(occtCellsBuildersOperandsA);
			kOcctShapeIteratorA.More();
			kOcctShapeIteratorA.Next())
		{
			for (BOPCol_ListIteratorOfListOfShape kOcctShapeIteratorB(occtCellsBuildersOperandsB);
				kOcctShapeIteratorB.More();
				kOcctShapeIteratorB.Next())
			{
				occtListToTake.Clear();
				occtListToAvoid.Clear();
				occtListToTake.Append(kOcctShapeIteratorA.Value());
				occtListToTake.Append(kOcctShapeIteratorB.Value());
				occtCellsBuilder.AddToResult(occtListToTake, occtListToAvoid);
			}
		}

		for (BOPCol_ListIteratorOfListOfShape kOcctShapeIteratorA(occtCellsBuildersOperandsA);
			kOcctShapeIteratorA.More();
			kOcctShapeIteratorA.Next())
		{
			occtListToTake.Clear();
			occtListToAvoid.Clear();
			occtListToTake.Append(kOcctShapeIteratorA.Value());
			occtListToAvoid.Append(occtCellsBuildersOperandsB);
			occtCellsBuilder.AddToResult(occtListToTake, occtListToAvoid);
		}

		return GetBooleanResult(
			kpOtherTopology,
			occtCellsBuilder,
			occtMapFaceToFixedFaceA,
			occtMapFaceToFixedFaceB);
	}

	Topology::Ptr Topology::Intersection(const Topology::Ptr& kpOtherTopology)
	{
		BOPCol_ListOfShape occtCellsBuildersOperandsA;
		BOPCol_ListOfShape occtCellsBuildersOperandsB;
		BOPAlgo_CellsBuilder occtCellsBuilder;
		BOPCol_DataMapOfShapeShape occtMapFaceToFixedFaceA;
		BOPCol_DataMapOfShapeShape occtMapFaceToFixedFaceB;

		BooleanOperation(kpOtherTopology, occtCellsBuilder, occtCellsBuildersOperandsA, occtCellsBuildersOperandsB, occtMapFaceToFixedFaceA, occtMapFaceToFixedFaceB);

		BOPCol_ListOfShape occtListToTake;
		BOPCol_ListOfShape occtListToAvoid;

		for (BOPCol_ListIteratorOfListOfShape kOcctShapeIteratorA(occtCellsBuildersOperandsA);
			kOcctShapeIteratorA.More();
			kOcctShapeIteratorA.Next())
		{
			for (BOPCol_ListIteratorOfListOfShape kOcctShapeIteratorB(occtCellsBuildersOperandsB);
				kOcctShapeIteratorB.More();
				kOcctShapeIteratorB.Next())
			{
				occtListToTake.Clear();
				occtListToAvoid.Clear();
				occtListToTake.Append(kOcctShapeIteratorA.Value());
				occtListToTake.Append(kOcctShapeIteratorB.Value());
				occtCellsBuilder.AddToResult(occtListToTake, occtListToAvoid);
			}
		}

		return GetBooleanResult(
			kpOtherTopology,
			occtCellsBuilder,
			occtMapFaceToFixedFaceA,
			occtMapFaceToFixedFaceB);
	}

	Topology::Ptr Topology::Merge(const Topology::Ptr& kpOtherTopology)
	{
		BOPCol_ListOfShape occtCellsBuildersOperandsA;
		BOPCol_ListOfShape occtCellsBuildersOperandsB;
		BOPAlgo_CellsBuilder occtCellsBuilder;
		BOPCol_DataMapOfShapeShape occtMapFaceToFixedFaceA;
		BOPCol_DataMapOfShapeShape occtMapFaceToFixedFaceB;

		// 1. The basic operation using cells builder.
		BooleanOperation(kpOtherTopology, occtCellsBuilder, occtCellsBuildersOperandsA, occtCellsBuildersOperandsB, occtMapFaceToFixedFaceA, occtMapFaceToFixedFaceB);

		// 2. Select the parts to be included in the final result.
		BOPCol_ListOfShape occtListToTake;
		BOPCol_ListOfShape occtListToAvoid;

		for (BOPCol_ListIteratorOfListOfShape kOcctShapeIteratorA(occtCellsBuildersOperandsA);
			kOcctShapeIteratorA.More();
			kOcctShapeIteratorA.Next())
		{
			occtListToTake.Clear();
			occtListToAvoid.Clear();
			occtListToTake.Append(kOcctShapeIteratorA.Value());
			occtCellsBuilder.AddToResult(occtListToTake, occtListToAvoid);
		}

		for (BOPCol_ListIteratorOfListOfShape kOcctShapeIteratorB(occtCellsBuildersOperandsB);
			kOcctShapeIteratorB.More();
			kOcctShapeIteratorB.Next())
		{
			occtListToTake.Clear();
			occtListToAvoid.Clear();
			occtListToTake.Append(kOcctShapeIteratorB.Value());
			occtCellsBuilder.AddToResult(occtListToTake, occtListToAvoid);
		}

		// 3. Process and return the result.
		return GetBooleanResult(
			kpOtherTopology,
			occtCellsBuilder,
			occtMapFaceToFixedFaceA,
			occtMapFaceToFixedFaceB);
	}

	Topology::Ptr Topology::Merge()
	{
		// 1
		BOPCol_ListOfShape occtShapes;
		ImmediateMembers(GetOcctShape(), occtShapes);

		// 2
		BOPAlgo_CellsBuilder occtCellsBuilder;
		occtCellsBuilder.SetArguments(occtShapes);
		//occtCellsBuilder.SetFuzzyValue(kTolerance);
		try {
			occtCellsBuilder.Perform();
		}
		catch (Standard_Failure& e)
		{
			const char* str = e.GetMessageString();
			std::string stlStr(str);
		}
		catch (std::exception& e)
		{
			const char* str = e.what();
			std::string stlStr(str);
		}

		if (occtCellsBuilder.HasErrors() || occtCellsBuilder.HasWarnings())
		{
			std::ostringstream errorStream;
			occtCellsBuilder.DumpErrors(errorStream);
			std::ostringstream warningStream;
			occtCellsBuilder.DumpWarnings(warningStream);

			// Exit here and return occtShapes as a cluster.
			TopoDS_Compound occtCompound;
			BRep_Builder occtBuilder;
			occtBuilder.MakeCompound(occtCompound);
			for (TopTools_ListIteratorOfListOfShape occtShapeIterator(occtShapes);
				occtShapeIterator.More();
				occtShapeIterator.Next())
			{
				occtBuilder.Add(occtCompound, occtShapeIterator.Value());
			}
			return Topology::ByOcctShape(occtCompound);
		}
		occtCellsBuilder.AddAllToResult();
		// DEBUG
		//return Topology::ByOcctShape(occtCellsBuilder.Shape());

		// 2b. Get discarded faces from Cells Builder
		BOPCol_ListOfShape occtDiscardedFaces;
		TopoDS_Compound occtCompound;
		BRep_Builder occtBuilder;
		occtBuilder.MakeCompound(occtCompound);
		for (BOPCol_ListIteratorOfListOfShape occtFaceIterator(occtShapes);
			occtFaceIterator.More();
			occtFaceIterator.Next())
		{
			TopoDS_Shape& rCurrent = occtFaceIterator.Value();
			if (occtCellsBuilder.IsDeleted(rCurrent))
			{
				occtBuilder.Add(occtCompound, rCurrent);
				occtDiscardedFaces.Append(rCurrent);
			}
		}
		//return Topology::ByOcctShape(occtCompound);


		// 3. Get Face[] from Topology[]
		BOPCol_ListOfShape occtFaces;
		TopoDS_Compound occtCompound3;
		BRep_Builder occtBuilder3;
		occtBuilder3.MakeCompound(occtCompound3);
		for (TopExp_Explorer occtExplorer(occtCellsBuilder.Shape(), TopAbs_FACE); occtExplorer.More(); occtExplorer.Next())
		{
			const TopoDS_Shape& rkOcctCurrent = occtExplorer.Current();
			if (!occtFaces.Contains(rkOcctCurrent))
			{
				occtFaces.Append(rkOcctCurrent);
				occtBuilder3.Add(occtCompound3, rkOcctCurrent);
			}
		}
		//return Topology::ByOcctShape(occtCompound3);

		// 5. Topology = VolumeMaker(Face[])--> first result
		BOPAlgo_MakerVolume occtVolumeMaker;
		Standard_Boolean bRunParallel = Standard_False; /* parallel or single mode (the default value is FALSE)*/
		Standard_Boolean bIntersect = Standard_True; /* intersect or not the arguments (the default value is TRUE)*/
		Standard_Real aTol = 0.0; /* fuzzy option (default value is 0)*/
								  //
		occtVolumeMaker.SetArguments(occtFaces);
		occtVolumeMaker.SetRunParallel(bRunParallel);
		occtVolumeMaker.SetIntersect(bIntersect);
		occtVolumeMaker.SetFuzzyValue(aTol);
		//
		occtVolumeMaker.Perform(); //perform the operation
		if (occtVolumeMaker.HasErrors() || occtVolumeMaker.HasWarnings()) { //check error status
			std::ostringstream errorStream;
			occtVolumeMaker.DumpErrors(errorStream);
			/*throw std::exception(errorStream.str().c_str());*/
			std::ostringstream warningStream;
			occtVolumeMaker.DumpWarnings(warningStream);
			TopoDS_Compound occtCompound;
			BRep_Builder occtBuilder;
			occtBuilder.MakeCompound(occtCompound);
			for (TopTools_ListIteratorOfListOfShape iterator(occtShapes);
				iterator.More();
				iterator.Next())
			{
				occtBuilder.Add(occtCompound, iterator.Value());
			}
			return Topology::ByOcctShape(occtCompound);
			//throw std::exception();
		}
		//DEBUG
		//return Topology::ByOcctShape(occtVolumeMaker.Shape());

		// 6. Get discarded faces from VolumeMaker--> second result
		TopoDS_Compound occtCompound2;
		BRep_Builder occtBuilder2;
		occtBuilder2.MakeCompound(occtCompound2);
		for (BOPCol_ListIteratorOfListOfShape occtFaceIterator(occtFaces);
			occtFaceIterator.More();
			occtFaceIterator.Next())
		{
			TopoDS_Shape& rCurrent = occtFaceIterator.Value();
			if (occtVolumeMaker.IsDeleted(rCurrent))
			{
				occtDiscardedFaces.Append(rCurrent);
				occtBuilder2.Add(occtCompound2, rCurrent);
			}
		}
		//return Topology::ByOcctShape(occtCompound2);

		// 7. Get the rest from Topology[] --> third result
		BOPCol_ListOfShape occtOtherShapes;
		for (BOPCol_ListIteratorOfListOfShape occtShapeIterator(occtShapes);
			occtShapeIterator.More();
			occtShapeIterator.Next())
		{
			if (occtShapeIterator.Value().ShapeType() != TopAbs_FACE)
			{
				occtOtherShapes.Append(occtShapeIterator.Value());
			}
		}

		//DEBUG: Draw discarded faces
		TopoDS_Compound occtCompound4;
		BRep_Builder occtBuilder4;
		occtBuilder4.MakeCompound(occtCompound4);
		for (BOPCol_ListIteratorOfListOfShape occtFaceIterator(occtDiscardedFaces);
			occtFaceIterator.More();
			occtFaceIterator.Next())
		{
			TopoDS_Shape& rCurrent = occtFaceIterator.Value();
			occtBuilder4.Add(occtCompound4, rCurrent);
		}
		//return Topology::ByOcctShape(occtCompound4);

		// 8. Merge results #1 #2 #3
		BOPCol_ListOfShape occtFinalArguments;
		occtFinalArguments.Append(occtVolumeMaker.Shape());
		occtFinalArguments.Append(occtDiscardedFaces);
		occtFinalArguments.Append(occtOtherShapes);

		if (occtFinalArguments.Size() == 1)
		{
			return Topology::ByOcctShape(occtVolumeMaker.Shape());
		}
		BOPAlgo_CellsBuilder occtCellsBuilder2;
		occtCellsBuilder2.SetArguments(occtFinalArguments);
		try {
			occtCellsBuilder2.Perform();
		}
		catch (Standard_Failure& e)
		{
			const char* str = e.GetMessageString();
			std::string stlStr(str);
		}
		catch (std::exception& e)
		{
			const char* str = e.what();
			std::string stlStr(str);
		}

		if (occtCellsBuilder2.HasErrors())
		{
			std::ostringstream errorStream;
			occtCellsBuilder2.DumpErrors(errorStream);
			throw std::exception(errorStream.str().c_str());
		}

		occtCellsBuilder2.AddAllToResult();
		occtCellsBuilder2.MakeContainers();

		// 9. If there is still a discarded face, add to merge2Topology as a cluster.
		BOPCol_ListOfShape clusterCandidates;
		for (BOPCol_ListIteratorOfListOfShape merge2Topologies(occtFinalArguments);
			merge2Topologies.More();
			merge2Topologies.Next())
		{
			if (occtCellsBuilder2.IsDeleted(merge2Topologies.Value()) &&
				merge2Topologies.Value().ShapeType() == TopAbs_FACE) // currently only face
			{
				TopTools_ListOfShape modifiedShapes = occtCellsBuilder2.Modified(merge2Topologies.Value());
				TopTools_ListOfShape generatedShapes = occtCellsBuilder2.Generated(merge2Topologies.Value());
				clusterCandidates.Append(merge2Topologies.Value());
			}
		}
		if (clusterCandidates.Size() > 0)
		{
			ImmediateMembers(occtCellsBuilder2.Shape(), clusterCandidates);
			TopoDS_Compound occtFinalCompound;
			BRep_Builder occtFinalBuilder;
			occtFinalBuilder.MakeCompound(occtFinalCompound);
			for (BOPCol_ListIteratorOfListOfShape clusterCandidateIterator(clusterCandidates);
				clusterCandidateIterator.More();
				clusterCandidateIterator.Next())
			{
				occtFinalBuilder.Add(occtFinalCompound, clusterCandidateIterator.Value());
			}
			return Topology::ByOcctShape(occtFinalCompound);
		}

		// Otherwise, return the result final merge result
		return Topology::ByOcctShape(occtCellsBuilder2.Shape());
	}

	Topology::Ptr Topology::Slice(const Topology::Ptr& kpOtherTopology)
	{
		// Check dimensionality. The second operand must be of lower dimensionality.
		if (kpOtherTopology->GetType() != TOPOLOGY_CLUSTER && Dimensionality() <= kpOtherTopology->Dimensionality())
		{
			throw std::exception("Cannot perform slice. The second operand must be of lower dimensionality.");
		}

		BOPCol_ListOfShape occtCellsBuildersOperandsA;
		BOPCol_ListOfShape occtCellsBuildersOperandsB;
		BOPAlgo_CellsBuilder occtCellsBuilder;

		// Map the input faces of the topology to the 
		BOPCol_DataMapOfShapeShape occtMapFaceToFixedFaceA;
		BOPCol_DataMapOfShapeShape occtMapFaceToFixedFaceB;

		BooleanOperation(kpOtherTopology, occtCellsBuilder, occtCellsBuildersOperandsA, occtCellsBuildersOperandsB, occtMapFaceToFixedFaceA, occtMapFaceToFixedFaceB);

		BOPCol_ListOfShape occtListToTake;
		BOPCol_ListOfShape occtListToAvoid;

		for (BOPCol_ListIteratorOfListOfShape kOcctShapeIteratorA(occtCellsBuildersOperandsA);
			kOcctShapeIteratorA.More();
			kOcctShapeIteratorA.Next())
		{
			occtListToTake.Clear();
			occtListToAvoid.Clear();
			occtListToTake.Append(kOcctShapeIteratorA.Value());
			occtCellsBuilder.AddToResult(occtListToTake, occtListToAvoid);
		}

		Topology::Ptr pResult = GetBooleanResult(
			kpOtherTopology,
			occtCellsBuilder,
			occtMapFaceToFixedFaceA,
			occtMapFaceToFixedFaceB);

		return pResult;
	}

	void Topology::AddUnionInternalStructure(const TopoDS_Shape& rkOcctShape, BOPCol_ListOfShape& rUnionArguments)
	{
		TopAbs_ShapeEnum occtShapeType = rkOcctShape.ShapeType();
		Topology::Ptr pTopology = Topology::ByOcctShape(rkOcctShape);
		std::list<Face::Ptr> faces;
		// Cell complex -> faces not part of the envelope
		// Cell -> inner shells
		// Shell --> inner wires of the faces
		// Face --> inner wires
		// Wire --> n/a
		// Edge --> n/a
		// Vertex --> n/a
		if (occtShapeType == TopAbs_COMPOUND)
		{
			std::shared_ptr<Cluster> pCluster = Topology::Downcast<Cluster>(pTopology);
			std::list<Topology::Ptr> immediateMembers;
			pCluster->ImmediateMembers(immediateMembers);
			for (const Topology::Ptr& kpImmediateMember : immediateMembers)
			{
				AddUnionInternalStructure(kpImmediateMember->GetOcctShape(), rUnionArguments);
			}
		} else if (occtShapeType == TopAbs_COMPSOLID)
		{
			CellComplex::Ptr pCellComplex = Topology::Downcast<CellComplex>(pTopology);
			pCellComplex->InnerBoundaries(faces);
			for (const Face::Ptr& kpInternalFace : faces)
			{
				rUnionArguments.Append(kpInternalFace->GetOcctShape());
			}
		}
		else if (occtShapeType == TopAbs_SOLID)
		{
			Cell::Ptr pCell = Topology::Downcast<Cell>(pTopology);
			std::list<Shell::Ptr> shells;
			pCell->InnerBoundaries(shells);
			for (const Shell::Ptr& kpInternalShell : shells)
			{
				rUnionArguments.Append(kpInternalShell->GetOcctShape());
			}
		}
		else if (occtShapeType == TopAbs_SHELL)
		{
			for (TopExp_Explorer occtShellExplorer(rkOcctShape, TopAbs_FACE); occtShellExplorer.More(); occtShellExplorer.Next())
			{
				const TopoDS_Shape& rkOcctCurrentFace = occtShellExplorer.Current();
				TopoDS_Wire occtOuterWire = BRepTools::OuterWire(TopoDS::Face(rkOcctCurrentFace));

				for (TopExp_Explorer occtFaceExplorer(rkOcctShape, TopAbs_WIRE); occtFaceExplorer.More(); occtFaceExplorer.Next())
				{
					const TopoDS_Shape& rkOcctCurrentFace = occtFaceExplorer.Current();
					if (!rUnionArguments.Contains(rkOcctCurrentFace) && !rkOcctCurrentFace.IsSame(occtOuterWire))
					{
						rUnionArguments.Append(rkOcctCurrentFace);
					}
				}
			}
		}
		else if (occtShapeType == TopAbs_FACE)
		{
			TopoDS_Wire occtOuterWire = BRepTools::OuterWire(TopoDS::Face(rkOcctShape));

			for (TopExp_Explorer occtExplorer(rkOcctShape, TopAbs_WIRE); occtExplorer.More(); occtExplorer.Next())
			{
				const TopoDS_Shape& occtCurrent = occtExplorer.Current();
				if (!rUnionArguments.Contains(occtCurrent) && !occtCurrent.IsSame(occtOuterWire))
				{
					rUnionArguments.Append(occtCurrent);
				}
			}
		}
	}

	TopoDS_Shape Topology::FixBooleanOperandCell(const TopoDS_Shape& rkOcctShape)
	{
		TopTools_MapOfShape occtCells;
		TopoDS_Shape occtNewShape(rkOcctShape);
		for (TopExp_Explorer occtExplorer(rkOcctShape, TopAbs_SOLID); occtExplorer.More(); occtExplorer.Next())
		{
			const TopoDS_Solid &occtCurrentSolid = TopoDS::Solid(occtExplorer.Current());
			//create tools for fixing a face 
			Handle(ShapeFix_Solid) occtShapeFixSolid = new ShapeFix_Solid();

			// create tool for rebuilding a shape and initialize it by shape 
			Handle(ShapeBuild_ReShape) occtContext = new ShapeBuild_ReShape();
			occtContext->Apply(occtNewShape);

			//set a tool for rebuilding a shape in the tool for fixing 
			occtShapeFixSolid->SetContext(occtContext);

			//initialize the fixing tool by one face 
			occtShapeFixSolid->Init(occtCurrentSolid);
			//fix the set face 
			occtShapeFixSolid->Perform();

			//get the result 
			occtNewShape = occtContext->Apply(occtNewShape);
		}

		return occtNewShape;
	}

	TopoDS_Shape Topology::FixBooleanOperandShell(const TopoDS_Shape& rkOcctShape)
	{
		TopTools_MapOfShape occtCells;
		TopoDS_Shape occtNewShape(rkOcctShape);
		for (TopExp_Explorer occtExplorer(rkOcctShape, TopAbs_SHELL); occtExplorer.More(); occtExplorer.Next())
		{
			const TopoDS_Shell& rkOcctCurrentShell = TopoDS::Shell(occtExplorer.Current());
			//create tools for fixing a face 
			Handle(ShapeFix_Shell) occtShapeFixShell = new ShapeFix_Shell();

			// create tool for rebuilding a shape and initialize it by shape 
			Handle(ShapeBuild_ReShape) occtContext = new ShapeBuild_ReShape();
			occtContext->Apply(occtNewShape);

			//set a tool for rebuilding a shape in the tool for fixing 
			occtShapeFixShell->SetContext(occtContext);

			//initialize the fixing tool by one face 
			occtShapeFixShell->Init(rkOcctCurrentShell);
			//fix the set face 
			occtShapeFixShell->Perform();

			//get the result 
			occtNewShape = occtContext->Apply(occtNewShape);
		}

		return occtNewShape;
	}

	TopoDS_Shape Topology::FixBooleanOperandFace(const TopoDS_Shape& rkOcctShape, BOPCol_DataMapOfShapeShape& rMapFaceToFixedFace)
	{
		TopTools_MapOfShape occtCells;
		TopoDS_Shape occtNewShape(rkOcctShape);
		for (TopExp_Explorer occtExplorer(rkOcctShape, TopAbs_FACE); occtExplorer.More(); occtExplorer.Next())
		{
			const TopoDS_Face& occtCurrentFace = TopoDS::Face(occtExplorer.Current());

			//create tools for fixing a face 
			Handle(ShapeFix_Face) occtShapeFixFace = new ShapeFix_Face();

			// create tool for rebuilding a shape and initialize it by shape 
			Handle(ShapeBuild_ReShape) occtContext = new ShapeBuild_ReShape();
			occtContext->Apply(occtNewShape);

			//set a tool for rebuilding a shape in the tool for fixing 
			occtShapeFixFace->SetContext(occtContext);

			//initialize the fixing tool by one face 
			occtShapeFixFace->Init(occtCurrentFace);
			//fix the set face 
			occtShapeFixFace->Perform();

			// Map occtCurrentFace and occtShapeFixFace.Shape()
			rMapFaceToFixedFace.Bind(occtCurrentFace, occtShapeFixFace->Face());

			//get the result 
			occtNewShape = occtContext->Apply(occtNewShape);
		}

		return occtNewShape;
	}

	//void Topology::RegisterLabel()
	//{
	//	LabelManager::GetInstance().AddLabelToRoot(GetOcctLabel());

	//	// Add the essential attributes:
	//	// - Shape
	//	LabelManager::AddShapeToLabel(GetOcctShape(), GetOcctLabel());

	//	// - Counter
	//	LabelManager::AddCounterToLabel(GetOcctLabel());

	//	// - Contents and contexts
	//	LabelManager::AddContentsContextsToLabel(GetOcctLabel());
	//}

	//void Topology::DecreaseCounter()
	//{
	//	LabelManager::GetInstance().DecreaseCounter(GetOcctLabel());
	//}

	void Topology::AddBooleanOperands(
		const Topology::Ptr& kpOtherTopology,
		BOPAlgo_CellsBuilder& rOcctCellsBuilder,
		BOPCol_ListOfShape& rOcctCellsBuildersOperandsA,
		BOPCol_ListOfShape& rOcctCellsBuildersOperandsB,
		BOPCol_DataMapOfShapeShape& rOcctMapFaceToFixedFaceA,
		BOPCol_DataMapOfShapeShape& rOcctMapFaceToFixedFaceB)
	{
		// Buffer lists are currently provided for cell complexes to be processed per cells.
		BOPCol_ListOfShape occtCellsBuildersArguments;
		TopologyType typeA = GetType();
		if (typeA == TOPOLOGY_CELLCOMPLEX || typeA == TOPOLOGY_CLUSTER)
		{
			std::list<Topology::Ptr> members;
			ImmediateMembers(members);
			for (const Topology::Ptr& kpMember : members)
			{
				TopoDS_Shape occtNewShape = FixBooleanOperandFace(kpMember->GetOcctShape(), rOcctMapFaceToFixedFaceA);
				occtNewShape = FixBooleanOperandShell(kpMember->GetOcctShape());
				occtNewShape = FixBooleanOperandCell(kpMember->GetOcctShape());
				rOcctCellsBuildersOperandsA.Append(kpMember->GetOcctShape());
				occtCellsBuildersArguments.Append(kpMember->GetOcctShape());
			}
		}
		else
		{
			BOPCol_DataMapOfShapeShape occtMapFaceToFixedFace;
			TopoDS_Shape occtNewShape = FixBooleanOperandFace(GetOcctShape(), rOcctMapFaceToFixedFaceA);
			occtNewShape = FixBooleanOperandShell(occtNewShape);
			occtNewShape = FixBooleanOperandCell(occtNewShape);
			rOcctCellsBuildersOperandsA.Append(occtNewShape);
			occtCellsBuildersArguments.Append(occtNewShape);
		}

		TopologyType typeB = kpOtherTopology->GetType();
		if (typeB == TOPOLOGY_CELLCOMPLEX || typeB == TOPOLOGY_CLUSTER)
		{
			std::list<Topology::Ptr> members;
			kpOtherTopology->ImmediateMembers(members);
			for (const Topology::Ptr& kpMember : members)
			{
				TopoDS_Shape occtNewShape = FixBooleanOperandFace(kpMember->GetOcctShape(), rOcctMapFaceToFixedFaceA);
				occtNewShape = FixBooleanOperandShell(kpMember->GetOcctShape());
				occtNewShape = FixBooleanOperandCell(kpMember->GetOcctShape());
				rOcctCellsBuildersOperandsB.Append(kpMember->GetOcctShape());
				occtCellsBuildersArguments.Append(kpMember->GetOcctShape());
			}
		}
		else
		{
			TopoDS_Shape occtNewShape = FixBooleanOperandFace(kpOtherTopology->GetOcctShape(), rOcctMapFaceToFixedFaceB);
			occtNewShape = FixBooleanOperandShell(kpOtherTopology->GetOcctShape());
			occtNewShape = FixBooleanOperandCell(kpOtherTopology->GetOcctShape());
			rOcctCellsBuildersOperandsB.Append(occtNewShape);
			occtCellsBuildersArguments.Append(occtNewShape);
		}

		rOcctCellsBuilder.SetArguments(occtCellsBuildersArguments);
	}

	Topology::Ptr Topology::Union(const Topology::Ptr& kpOtherTopology)
	{
		BOPCol_ListOfShape occtCellsBuildersOperandsA;
		BOPCol_ListOfShape occtCellsBuildersOperandsB;
		BOPAlgo_CellsBuilder occtCellsBuilder;
		BOPCol_DataMapOfShapeShape occtMapFaceToFixedFaceA;
		BOPCol_DataMapOfShapeShape occtMapFaceToFixedFaceB;

		BooleanOperation(kpOtherTopology, occtCellsBuilder, occtCellsBuildersOperandsA, occtCellsBuildersOperandsB, occtMapFaceToFixedFaceA, occtMapFaceToFixedFaceB);

		BOPCol_ListOfShape occtListToTake;
		BOPCol_ListOfShape occtListToAvoid;

		for (BOPCol_ListIteratorOfListOfShape kOcctShapeIterator(occtCellsBuildersOperandsA);
			kOcctShapeIterator.More();
			kOcctShapeIterator.Next())
		{
			occtListToTake.Clear();
			occtListToAvoid.Clear();
			occtListToTake.Append(kOcctShapeIterator.Value());
			occtCellsBuilder.AddToResult(occtListToTake, occtListToAvoid, 1, true);
		}

		for (BOPCol_ListIteratorOfListOfShape kOcctShapeIterator(occtCellsBuildersOperandsB);
			kOcctShapeIterator.More();
			kOcctShapeIterator.Next())
		{
			occtListToTake.Clear();
			occtListToAvoid.Clear();
			occtListToTake.Append(kOcctShapeIterator.Value());
			occtCellsBuilder.AddToResult(occtListToTake, occtListToAvoid, 1, true);
		}

		Topology::Ptr pTopology = GetBooleanResult(
			kpOtherTopology,
			occtCellsBuilder,
			occtMapFaceToFixedFaceA,
			occtMapFaceToFixedFaceB);

		BOPAlgo_CellsBuilder occtCellsBuilder2;
		BOPCol_ListOfShape occtCellsBuildersArguments;
		occtCellsBuildersArguments.Append(pTopology->GetOcctShape());

		// Get the internal boundaries
		AddUnionInternalStructure(GetOcctShape(), occtCellsBuildersArguments);
		AddUnionInternalStructure(kpOtherTopology->GetOcctShape(), occtCellsBuildersArguments);
		
		if (occtCellsBuildersArguments.Size() < 2)
		{
			return Topology::ByOcctShape(occtCellsBuildersArguments.First());
		}

		occtCellsBuilder2.SetArguments(occtCellsBuildersArguments);
		occtCellsBuilder2.Perform();

		if (occtCellsBuilder2.HasErrors())
		{
			std::ostringstream errorStream;
			occtCellsBuilder2.DumpErrors(errorStream);
			throw std::exception(errorStream.str().c_str());
		}
		occtCellsBuilder2.AddAllToResult();
		TopoDS_Shape occtUnionResult = occtCellsBuilder2.Shape();
		Topology::Ptr pUnionTopology = Topology::ByOcctShape(occtUnionResult);
		return pUnionTopology;
	}

	Topology::Ptr Topology::XOR(const Topology::Ptr& kpOtherTopology)
	{
		BOPCol_ListOfShape occtCellsBuildersOperandsA;
		BOPCol_ListOfShape occtCellsBuildersOperandsB;
		BOPAlgo_CellsBuilder occtCellsBuilder;
		BOPCol_DataMapOfShapeShape occtMapFaceToFixedFaceA;
		BOPCol_DataMapOfShapeShape occtMapFaceToFixedFaceB;

		BooleanOperation(kpOtherTopology, occtCellsBuilder, occtCellsBuildersOperandsA, occtCellsBuildersOperandsB, occtMapFaceToFixedFaceA, occtMapFaceToFixedFaceB);

		BOPCol_ListOfShape occtListToTake;
		BOPCol_ListOfShape occtListToAvoid;
		for (BOPCol_ListIteratorOfListOfShape kOcctShapeIteratorA(occtCellsBuildersOperandsA);
			kOcctShapeIteratorA.More();
			kOcctShapeIteratorA.Next())
		{
			occtListToTake.Clear();
			occtListToAvoid.Clear();
			occtListToTake.Append(kOcctShapeIteratorA.Value());
			for (BOPCol_ListIteratorOfListOfShape kOcctShapeIteratorB(occtCellsBuildersOperandsB);
				kOcctShapeIteratorB.More();
				kOcctShapeIteratorB.Next())
			{
				occtListToAvoid.Append(kOcctShapeIteratorB.Value());
			}
			occtCellsBuilder.AddToResult(occtListToTake, occtListToAvoid);
		}


		for (BOPCol_ListIteratorOfListOfShape kOcctShapeIteratorB(occtCellsBuildersOperandsB);
			kOcctShapeIteratorB.More();
			kOcctShapeIteratorB.Next())
		{
			occtListToTake.Clear();
			occtListToAvoid.Clear();
			occtListToTake.Append(kOcctShapeIteratorB.Value());
			for (BOPCol_ListIteratorOfListOfShape kOcctShapeIteratorA(occtCellsBuildersOperandsA);
				kOcctShapeIteratorA.More();
				kOcctShapeIteratorA.Next())
			{
				occtListToAvoid.Append(kOcctShapeIteratorA.Value());
			}
			occtCellsBuilder.AddToResult(occtListToTake, occtListToAvoid);
		}

		return GetBooleanResult(
			kpOtherTopology,
			occtCellsBuilder,
			occtMapFaceToFixedFaceA,
			occtMapFaceToFixedFaceB);
	}

	void Topology::ImmediateMembers(const TopoDS_Shape& rkShape, BOPCol_ListOfShape& rImmediateMembers)
	{
		for (TopoDS_Iterator occtShapeIterator(rkShape); occtShapeIterator.More(); occtShapeIterator.Next())
		{
			rImmediateMembers.Append(occtShapeIterator.Value());
		}
	}

	void Topology::ImmediateMembers(std::list<Topology::Ptr>& rImmediateMembers) const
	{
		BOPCol_ListOfShape occtListMembers;
		Topology::ImmediateMembers(GetOcctShape(), occtListMembers);
		for (BOPCol_ListIteratorOfListOfShape occtIterator(occtListMembers);
			occtIterator.More();
			occtIterator.Next())
		{
			Topology::Ptr pMemberTopology = Topology::ByOcctShape(occtIterator.Value());

			//if (!GetOcctLabel().IsNull())
			//{
			//	// Does this topology have a label in the hierachy under the topology? If yes, set the label.
			//	for (TDF_ChildIterator occtLabelIterator(GetOcctLabel(), true); occtLabelIterator.More(); occtLabelIterator.Next())
			//	{
			//		TDF_Label childLabel = occtLabelIterator.Value();
			//		Handle(TNaming_NamedShape) occtChildShape;
			//		Handle(TDataStd_Integer) occtRelationshipType;
			//		bool result1 = childLabel.FindAttribute(TNaming_NamedShape::GetID(), occtChildShape);
			//		bool result2 = childLabel.FindAttribute(TDataStd_Integer::GetID(), occtRelationshipType);
			//		int result3 = occtRelationshipType->Get();
			//		if (result1 &&
			//			result2 &&
			//			occtRelationshipType->Get() == REL_CONSTITUENT &&
			//			occtChildShape->Get().IsSame(occtIterator.Value()))
			//		{
			//			pMemberTopology->SetOcctLabel(childLabel);
			//		}
			//	}
			//}

			rImmediateMembers.push_back(pMemberTopology);
		}
	}

	void Topology::UpwardNavigation(const TopoDS_Shape & rkOcctShape, const TopoDS_Shape & rkOcctParentShape, const TopAbs_ShapeEnum kShapeEnum, TopTools_ListOfShape & rOcctMembers)
	{
		TopTools_ListOfShape occtAncestors;
		TopTools_IndexedDataMapOfShapeListOfShape occtShapeMap;
		TopExp::MapShapesAndUniqueAncestors(
			rkOcctShape,
			rkOcctShape.ShapeType(),
			kShapeEnum,
			occtShapeMap);

		occtShapeMap.FindFromKey(rkOcctShape, occtAncestors);
	}

	void Topology::DownwardNavigation(const TopoDS_Shape& rkOcctShape, const TopAbs_ShapeEnum & rkShapeEnum, TopTools_ListOfShape & rOcctMembers)
	{
		for (TopExp_Explorer occtExplorer(rkOcctShape, rkShapeEnum); occtExplorer.More(); occtExplorer.Next())
		{
			const TopoDS_Shape& occtCurrent = occtExplorer.Current();
			if (!rOcctMembers.Contains(occtCurrent))
			{
				rOcctMembers.Append(occtCurrent);
			}
		}
	}

	/*void Topology::AddIngredientTo(const Topology::Ptr& kpTopology)
	{
		if(!IsIngredientTo(kpTopology))
		{
			m_ingredientTo.push_back(kpTopology);
			kpTopology->m_ingredients.push_back(shared_from_this());

			GlobalCluster::GetInstance().Remove(this);
		}
	}

	void Topology::RemoveIngredientTo(const Topology::Ptr& kpTopology)
	{
		m_ingredientTo.remove(kpTopology);
		kpTopology->m_ingredients.remove(shared_from_this());
		if (m_ingredientTo.empty())
		{
			GlobalCluster::GetInstance().Add(this);
		}
	}

	bool Topology::IsIngredientTo(const Topology::Ptr& kpTopology) const
	{
		return std::find(m_ingredientTo.begin(), m_ingredientTo.end(), kpTopology) != m_ingredientTo.end();
	}

	bool Topology::IsIngredient() const
	{
		return !m_ingredientTo.empty();
	}*/

	bool Topology::IsSame(const Topology::Ptr& kpTopology) const
	{
		bool isSame = GetOcctShape().IsSame(kpTopology->GetOcctShape());
		return isSame;
	}

	void Topology::Members(TopTools_ListOfShape& rOcctMembers) const
	{
		const TopoDS_Shape& rkOcctShape = GetOcctShape();
		// Store the children
		for (int i = ((int)rkOcctShape.ShapeType()) + 1; i < (int)TopAbs_SHAPE; ++i)
		{
			TopAbs_ShapeEnum occtShapeEnum = (TopAbs_ShapeEnum)i;
			TopTools_ListOfShape occtMembers;
			Topology::DownwardNavigation(rkOcctShape, occtShapeEnum, occtMembers);
			for (TopTools_ListIteratorOfListOfShape occtMembersIterator(occtMembers);
				occtMembersIterator.More();
				occtMembersIterator.Next())
			{
				rOcctMembers.Append(occtMembersIterator.Value());
			}
		}
	}

	void Topology::Members(std::list<Topology::Ptr>& rMembers) const
	{
		TopTools_ListOfShape occtMembers;
		Members(occtMembers);
		for (TopTools_ListIteratorOfListOfShape occtMemberIterator(occtMembers); occtMemberIterator.More(); occtMemberIterator.Next())
		{
			/*TDF_Label occtLabel;
			LabelManager::GetInstance().FindLabelByShape(occtMemberIterator.Value(), occtLabel);
			rMembers.push_back(Topology::ByOcctShape(occtMemberIterator.Value(), occtLabel));*/
			rMembers.push_back(Topology::ByOcctShape(occtMemberIterator.Value()));
		}
	}

	/*void Topology::AddIngredient(Topology * const kpTopology)
	{
		m_ingredients.push_back(kpTopology);
	}

	void Topology::RemoveIngredient(Topology * const kpTopology)
	{
		m_ingredients.remove(kpTopology);
	}*/
}