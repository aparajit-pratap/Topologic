#pragma once

#include "Vertex.h"

namespace Topologic {
	namespace Utilities {


		/// <summary>
		/// VertexUtility includes geometric methods relevant to a Vertex.
		/// </summary>
		public ref class VertexUtility
		{
		public:
			/// <summary>
			/// Returns a list of Edges that are the adjacent to the input Vertex.
			/// </summary>
			/// <param name="vertex">A Vertex</param>
			/// <param name="parentTopology">A parent Topology</param>
			/// <returns name="Edge[]">A list of Edges adjacent to the input Vertex</returns>
			static List<Edge^>^ AdjacentEdges(Vertex^ vertex, Topology^ parentTopology);

		public protected:
			VertexUtility() {}
		};
	}
}