#pragma once

#include <Topology.h>

#include <TopoLogicCore/include/Aperture.h>

using namespace System::Collections::Generic;

namespace TopoLogic
{
	ref class Context;

	/// <summary>
	/// <para>
	/// An Aperture defines a connection path between two Topologies that share a common Topology or between a
	/// Topology and the outside space. A connection path can be uni-directional or bi-directional.
	/// </para>
	/// </summary>
	public ref class Aperture : public Topology
	{
	public:
		/// <summary>
		/// Creates an aperture by a topology and a context.
		/// </summary>
		/// <param name="topology">A topology</param>
		/// <param name="context">A context</param>
		/// <returns name="Aperture">An aperture</returns>
		static Aperture^ ByTopologyContext(Topology^ topology, Context^ context);

		/// <summary>
		/// Creates an aperture by a topology, a context, and an open status.
		/// </summary>
		/// <param name="topology">A topology</param>
		/// <param name="context">A context</param>
		/// <param name="openStatus">An open status</param>
		/// <returns name="Aperture">An aperture</returns>
		static Aperture^ ByTopologyContextStatus(Topology^ topology, Context^ context, bool openStatus);

		/// <summary>
		/// Returns the underlying topology.
		/// </summary>
		/// <returns name="Topology">The underlying topology</returns>
		TopoLogic::Topology^ Topology();

		/// <summary>
		/// Checks if any aperture path is open.
		/// </summary>
		/// <returns name="bool">True if the aperture is open</returns>
		bool IsOpen();

		/// <summary>
		/// Checks if the aperture path from the first to the second topologies is open.
		/// </summary>
		/// <param name="Topology[]">A list of exactly two topologies</param>
		/// <returns name="bool">True if the aperture between the two paths is open</returns>
		bool IsOpen(List<TopoLogic::Topology^>^ topologies);

		/// <summary>
		/// Returns all open paths.
		/// </summary>
		/// <returns name="Topology[][]">The open paths</returns>
		List<List<TopoLogic::Topology^>^>^ Paths();

		/// <summary>
		/// Open the aperture paths connecting all adjacent topologies.
		/// </summary>
		/// <returns name="Aperture">An aperture</returns>
		Aperture^ Open();

		/// <summary>
		/// Open the aperture path connecting the input topologies.
		/// </summary>
		/// <param name="Topology[]">Topologies</param>
		/// <returns name="Aperture">An aperture</returns>
		Aperture^ Open(List<TopoLogic::Topology^>^ topologies);

		/// <summary>
		/// Close all aperture paths.
		/// </summary>
		/// <returns name="Aperture"></returns>
		Aperture^ Close();

		/// <summary>
		/// Close the aperture path between the input topologies.
		/// </summary>
		/// <param name="Topology[]">Topologies</param>
		/// <returns name="Aperture">An aperture</returns>
		Aperture^ Close(List<TopoLogic::Topology^>^ topologies);

		property Object^ Geometry
		{
			/// <summary>
			/// 
			/// </summary>
			/// <returns name="Geometry"></returns>
			virtual Object^ get() override;
		}

	public protected:
		Aperture(const std::shared_ptr<TopoLogicCore::Aperture>& kpCoreAperture);

		virtual std::shared_ptr<TopoLogicCore::TopologicalQuery> GetCoreTopologicalQuery() override;

	protected:
		virtual ~Aperture();

		std::shared_ptr<TopoLogicCore::Aperture>* m_pCoreAperture;
	};
}