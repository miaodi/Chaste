/*

Copyright (c) 2005-2016, University of Oxford.
All rights reserved.

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
 * Neither the name of the University of Oxford nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef TESTBOXDOMAINPDEMODIFIERS_HPP_
#define TESTBOXDOMAINPDEMODIFIERS_HPP_

#include <cxxtest/TestSuite.h>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "CheckpointArchiveTypes.hpp"

#include "EllipticBoxDomainPdeModifier.hpp"
//#include "ParabolicBoxDomainPdeModifier.hpp"
#include "PdeAndBoundaryConditions.hpp"
#include "ParabolicPdeAndBoundaryConditions.hpp"

#include "HoneycombMeshGenerator.hpp"
#include "HoneycombVertexMeshGenerator.hpp"
#include "MutableVertexMesh.hpp"
#include "PottsMeshGenerator.hpp"
#include "PottsMesh.hpp"
#include "CellsGenerator.hpp"
#include "FixedDurationGenerationBasedCellCycleModel.hpp"
#include "VertexBasedCellPopulation.hpp"
#include "MeshBasedCellPopulation.hpp"
#include "NodeBasedCellPopulation.hpp"
#include "PottsBasedCellPopulation.hpp"
#include "CaBasedCellPopulation.hpp"
#include "CellwiseSourceEllipticPde.hpp"
#include "SimpleUniformSourcePde.hpp"
#include "UniformSourceParabolicPde.hpp"
#include "ConstBoundaryCondition.hpp"
#include "ReplicatableVector.hpp"
#include "WildTypeCellMutationState.hpp"
#include "AveragedSourcePde.hpp"
#include "FileComparison.hpp"
#include "NumericFileComparison.hpp"
#include "FunctionalBoundaryCondition.hpp"
#include "ArchiveOpener.hpp"
#include "SmartPointers.hpp"
#include "AbstractCellBasedWithTimingsTestSuite.hpp"

// This test is always run sequentially (never in parallel)
#include "FakePetscSetup.hpp"


class SimplePdeForTesting : public AbstractLinearEllipticPde<2,2>
{
public:
    double ComputeConstantInUSourceTerm(const ChastePoint<2>&, Element<2,2>* pElement)
    {
        return -1.0;
    }

    double ComputeLinearInUCoeffInSourceTerm(const ChastePoint<2>&, Element<2,2>*)
    {
        return 0.0;
    }

    c_matrix<double,2,2> ComputeDiffusionTerm(const ChastePoint<2>& )
    {
        return identity_matrix<double>(2);
    }
};

class TestBoxDomainPdeModifiers : public AbstractCellBasedWithTimingsTestSuite
{
public:
    void TestEllipticConstructor() throw(Exception)
    {
        // Make the Pde and BCS
        SimpleUniformSourcePde<2> pde(-0.1);
        ConstBoundaryCondition<2> bc(1.0);
        PdeAndBoundaryConditions<2> pde_and_bc(&pde, &bc, false);
        pde_and_bc.SetDependentVariableName("averaged quantity");

        // Make domain
        ChastePoint<2> lower(-10.0, -10.0);
        ChastePoint<2> upper(10.0, 10.0);
        ChasteCuboid<2> cuboid(lower, upper);

        // Create an Elliptic PDE Modifier object using this pde and bcs object
        MAKE_PTR_ARGS(EllipticBoxDomainPdeModifier<2>, p_pde_modifier, (&pde_and_bc, cuboid, 1.0));

        // Test that member variables are initialised correctly
        TS_ASSERT_EQUALS(p_pde_modifier->mpPdeAndBcs->rGetDependentVariableName(), "averaged quantity");

        // Check mesh
        TS_ASSERT_EQUALS(p_pde_modifier->mpFeMesh->GetNumNodes(),441u);
        TS_ASSERT_EQUALS(p_pde_modifier->mpFeMesh->GetNumBoundaryNodes(),80u);
        TS_ASSERT_EQUALS(p_pde_modifier->mpFeMesh->GetNumElements(),800u);
        TS_ASSERT_EQUALS(p_pde_modifier->mpFeMesh->GetNumBoundaryElements(),80u);

        ChasteCuboid<2> bounding_box = p_pde_modifier->mpFeMesh->CalculateBoundingBox();
        TS_ASSERT_DELTA(bounding_box.rGetUpperCorner()[0],10,1e-5);
        TS_ASSERT_DELTA(bounding_box.rGetUpperCorner()[1],10,1e-5);
        TS_ASSERT_DELTA(bounding_box.rGetLowerCorner()[0],-10,1e-5);
        TS_ASSERT_DELTA(bounding_box.rGetLowerCorner()[1],-10,1e-5);

        //Coverage
        TS_ASSERT_EQUALS(p_pde_modifier->GetOutputGradient(),false); //Defaults to false
        p_pde_modifier->SetOutputGradient(true);
        TS_ASSERT_EQUALS(p_pde_modifier->GetOutputGradient(),true);
    }

//    void TestParabolicConstructor() throw(Exception)
//    {
//        // Make the Pde and BCS
//        UniformSourceParabolicPde<2> pde(-0.1);
//        ConstBoundaryCondition<2> bc(1.0);
//        ParabolicPdeAndBoundaryConditions<2> pde_and_bc(&pde, &bc, false);
//        pde_and_bc.SetDependentVariableName("averaged quantity");
//
//        // Create a Parabolic PDE Modifier object using this pde and bcs object
//        MAKE_PTR_ARGS(ParabolicBoxDomainPdeModifier<2>, p_pde_modifier, (&pde_and_bc));
//
//        // Test that member variables are initialised correctly
//        TS_ASSERT_EQUALS(p_pde_modifier->mpPdeAndBcs->rGetDependentVariableName(), "averaged quantity");
//    }

//    void TestMeshGeneration() throw(Exception)
//    {
//        // Create a PDE and BCs object to be used by all cell populations
//        SimpleUniformSourcePde<2> pde(-0.1);
//        ConstBoundaryCondition<2> bc(1.0);
//        PdeAndBoundaryConditions<2> pde_and_bc(&pde, &bc, false);
//        pde_and_bc.SetDependentVariableName("averaged quantity");
//
//        // Create a CellsGenerator to be used by all cell populations
//        CellsGenerator<FixedDurationGenerationBasedCellCycleModel, 2> cells_generator;
//
//        // Create a PDE Modifier object using this PDE and BCs object
//        MAKE_PTR_ARGS(EllipticBoxDomainPdeModifier<2>, p_pde_modifier, (&pde_and_bc));
//        {
//            // Create a MeshBasedCellPopulation
//            HoneycombMeshGenerator generator(10,10, 0);
//            MutableMesh<2,2>* p_mesh = generator.GetMesh();
//
//            std::vector<CellPtr> mesh_cells;
//            cells_generator.GenerateBasic(mesh_cells, p_mesh->GetNumNodes());
//
//            MeshBasedCellPopulation<2> mesh_cell_population(*p_mesh, mesh_cells);
//
//            // Now generate the finite element mesh
//            p_pde_modifier->GenerateFeMesh(mesh_cell_population);
//
//            // Check that the meshes have the same nodes
//            for (unsigned i=0; i<p_mesh->GetNumNodes(); i++)
//            {
//                TS_ASSERT_DELTA(p_pde_modifier->mpFeMesh->GetNode(i)->rGetLocation()[0], p_mesh->GetNode(i)->rGetLocation()[0],1e-5);
//                TS_ASSERT_DELTA(p_pde_modifier->mpFeMesh->GetNode(i)->rGetLocation()[1], p_mesh->GetNode(i)->rGetLocation()[1],1e-5);
//
//                TS_ASSERT_EQUALS(p_pde_modifier->mpFeMesh->GetNode(i)->IsBoundaryNode(), p_mesh->GetNode(i)->IsBoundaryNode());
//            }
//        }
//
//        {
//            // Make a NodeBasedCellPopulation
//            HoneycombMeshGenerator generator(10, 10, 0);
//            MutableMesh<2,2>* p_mesh = generator.GetMesh();
//            NodesOnlyMesh<2> node_mesh;
//            node_mesh.ConstructNodesWithoutMesh(*p_mesh, 1.5);
//
//            std::vector<CellPtr> node_cells;
//            cells_generator.GenerateBasic(node_cells, node_mesh.GetNumNodes());
//
//            NodeBasedCellPopulation<2> node_cell_population(node_mesh, node_cells);
//
//            // Now generate the finite element mesh
//            p_pde_modifier->GenerateFeMesh(node_cell_population);
//
//            // Check that the meshes have the same nodes
//            for (unsigned i=0; i<node_mesh.GetNumNodes(); i++)
//            {
//                TS_ASSERT_DELTA(p_pde_modifier->mpFeMesh->GetNode(i)->rGetLocation()[0], node_mesh.GetNode(i)->rGetLocation()[0],1e-5);
//                TS_ASSERT_DELTA(p_pde_modifier->mpFeMesh->GetNode(i)->rGetLocation()[1], node_mesh.GetNode(i)->rGetLocation()[1],1e-5);
//            }
//        }
//
//        {
//            // Make a VertexBasedCellPopulation
//            HoneycombVertexMeshGenerator generator(10, 10);
//            MutableVertexMesh<2,2>* p_mesh = generator.GetMesh();
//
//            std::vector<CellPtr> vertex_cells;
//            cells_generator.GenerateBasic(vertex_cells, p_mesh->GetNumElements());
//
//            VertexBasedCellPopulation<2> vertex_cell_population(*p_mesh, vertex_cells);
//
//            // Now generate the finite element mesh
//            p_pde_modifier->GenerateFeMesh(vertex_cell_population);
//
//            // Check that the meshes have the same nodes
//            for (unsigned i=0; i<p_mesh->GetNumNodes(); i++)
//            {
//                TS_ASSERT_DELTA(p_pde_modifier->mpFeMesh->GetNode(i)->rGetLocation()[0], p_mesh->GetNode(i)->rGetLocation()[0],1e-5);
//                TS_ASSERT_DELTA(p_pde_modifier->mpFeMesh->GetNode(i)->rGetLocation()[1], p_mesh->GetNode(i)->rGetLocation()[1],1e-5);
//
//                TS_ASSERT_EQUALS(p_pde_modifier->mpFeMesh->GetNode(i)->IsBoundaryNode(), p_mesh->GetNode(i)->IsBoundaryNode());
//            }
//            // New Node at every element centre
//            for (unsigned i=0; i<p_mesh->GetNumElements(); i++)
//            {
//                TS_ASSERT_DELTA(p_pde_modifier->mpFeMesh->GetNode(i+p_mesh->GetNumNodes())->rGetLocation()[0], p_mesh->GetCentroidOfElement(i)[0],1e-5);
//                TS_ASSERT_DELTA(p_pde_modifier->mpFeMesh->GetNode(i+p_mesh->GetNumNodes())->rGetLocation()[1], p_mesh->GetCentroidOfElement(i)[1],1e-5);
//
//                TS_ASSERT_EQUALS(p_pde_modifier->mpFeMesh->GetNode(i+p_mesh->GetNumNodes())->IsBoundaryNode(), false);
//            }
//        }
//
//        {
//            // Make a PottsBasedCellPopulation
//            PottsMeshGenerator<2> generator(50,5,5,50,5,5);
//            PottsMesh<2>* p_mesh = generator.GetMesh();
//
//            std::vector<CellPtr> potts_cells;
//            cells_generator.GenerateBasic(potts_cells, p_mesh->GetNumElements());
//
//            PottsBasedCellPopulation<2> potts_cell_population(*p_mesh, potts_cells);
//
//            // Now generate the finite element mesh
//            p_pde_modifier->GenerateFeMesh(potts_cell_population);
//
//            // Check that the meshes have the same nodes
//            for (unsigned i=0; i<p_mesh->GetNumElements(); i++)
//            {
//                TS_ASSERT_DELTA(p_pde_modifier->mpFeMesh->GetNode(i)->rGetLocation()[0], p_mesh->GetCentroidOfElement(i)[0],1e-5);
//                TS_ASSERT_DELTA(p_pde_modifier->mpFeMesh->GetNode(i)->rGetLocation()[1], p_mesh->GetCentroidOfElement(i)[1],1e-5);
//            }
//        }
//
//        {
//            // Make a CaBasedCellPopulation
//            PottsMeshGenerator<2> generator(50,0,0,50,0,0);
//            PottsMesh<2>* p_mesh = generator.GetMesh();
//
//            // Specify the location of each cell
//            std::vector<unsigned> location_indices;
//            for (unsigned i=0; i<10; i++)
//            {
//                for (unsigned j=0; j<10; j++)
//                {
//                    unsigned offset = (50+1) * (50-10)/2;
//                    location_indices.push_back(offset + j + i * 50);
//                }
//            }
//
//            std::vector<CellPtr> ca_cells;
//            cells_generator.GenerateBasic(ca_cells, location_indices.size());
//
//            // Create cell population
//            CaBasedCellPopulation<2> ca_cell_population(*p_mesh, ca_cells, location_indices);
//
//            // Now generate the finite element mesh
//            p_pde_modifier->GenerateFeMesh(ca_cell_population);
//
//            // Check that the mesh has nodes at the centre of the cells
//            unsigned i=0;
//            for (AbstractCellPopulation<2>::Iterator cell_iter = ca_cell_population.Begin();
//                 cell_iter != ca_cell_population.End();
//                 ++cell_iter)
//            {
//                TS_ASSERT_DELTA(p_pde_modifier->mpFeMesh->GetNode(i)->rGetLocation()[0], ca_cell_population.GetLocationOfCellCentre(*cell_iter)[0],1e-5);
//                TS_ASSERT_DELTA(p_pde_modifier->mpFeMesh->GetNode(i)->rGetLocation()[1], ca_cell_population.GetLocationOfCellCentre(*cell_iter)[1],1e-5);
//                ++i;
//            }
//        }
//    }

    void TestArchiveEllipticBoxDomainPdeModifier() throw(Exception)
    {
        // Create a file for archiving
        OutputFileHandler handler("archive", false);
        handler.SetArchiveDirectory();
        std::string archive_filename = handler.GetOutputDirectoryFullPath() + "EllipticBoxDomainPdeModifier.arch";

        // Separate scope to write the archive
        {
            // Make the Pde and BCS
            SimpleUniformSourcePde<2> pde(-0.1);
            ConstBoundaryCondition<2> bc(1.0);
            PdeAndBoundaryConditions<2> pde_and_bc(&pde, &bc, false);
            pde_and_bc.SetDependentVariableName("averaged quantity");

            // Make domain
            ChastePoint<2> lower(-10.0, -10.0);
            ChastePoint<2> upper(10.0, 10.0);
            ChasteCuboid<2> cuboid(lower, upper);

            // Initialise an Elliptic PDE Modifier object using this pde and bcs object
            AbstractCellBasedSimulationModifier<2,2>* const p_modifier = new EllipticBoxDomainPdeModifier<2>(&pde_and_bc, cuboid, 2.0);

            // Create an output archive
            std::ofstream ofs(archive_filename.c_str());
            boost::archive::text_oarchive output_arch(ofs);

            // Serialize via pointer
            output_arch << p_modifier;
            delete p_modifier;
        }

        // Separate scope to read the archive
        {
            AbstractCellBasedSimulationModifier<2,2>* p_modifier2;

            // Restore the modifier
            std::ifstream ifs(archive_filename.c_str());
            boost::archive::text_iarchive input_arch(ifs);

            input_arch >> p_modifier2;

            // See whether we read out the correct variable name area
            std::string variable_name = (static_cast<EllipticBoxDomainPdeModifier<2>*>(p_modifier2))->mpPdeAndBcs->rGetDependentVariableName();
            // Test that member variables are initialised correctly
            TS_ASSERT_EQUALS(variable_name, "averaged quantity");

            ///\todo #2687 The archive created a new PDE object. Memory-management of mpPdeAndBcs is not enabled. Suggest using a shared-pointer.
            delete (static_cast<EllipticBoxDomainPdeModifier<2>*>(p_modifier2))->mpPdeAndBcs;
            delete p_modifier2;
        }
    }


//    void TestArchiveParabolicBoxDomainPdeModifier() throw(Exception)
//    {
//        // Create a file for archiving
//        OutputFileHandler handler("archive", false);
//        handler.SetArchiveDirectory();
//        std::string archive_filename = handler.GetOutputDirectoryFullPath() + "ParabolicBoxDomainPdeModifier.arch";
//
//        // Separate scope to write the archive
//        {
//            // Make the Pde and BCS
//            UniformSourceParabolicPde<2> pde(-0.1);
//            ConstBoundaryCondition<2> bc(1.0);
//            ParabolicPdeAndBoundaryConditions<2> pde_and_bc(&pde, &bc, false);
//            pde_and_bc.SetDependentVariableName("averaged quantity");
//
//            ///\todo #2687
//            // Make a dummy solution (because the archiver expects to be able to read/write PETSc Vecs).
//            Vec vector = PetscTools::CreateAndSetVec(10, -42.0);
//            pde_and_bc.SetSolution(vector);
//
//            // Initialise a Parabolic PDE Modifier object using this pde and bcs object
//            AbstractCellBasedSimulationModifier<2,2>* const p_modifier = new ParabolicBoxDomainPdeModifier<2>(&pde_and_bc);
//
//            // Create an output archive
//            std::ofstream ofs(archive_filename.c_str());
//            boost::archive::text_oarchive output_arch(ofs);
//
//            // Serialize via pointer
//            output_arch << p_modifier;
//            delete p_modifier;
//        }
//
//        // Separate scope to read the archive
//        {
//            AbstractCellBasedSimulationModifier<2,2>* p_modifier2;
//
//            // Restore the modifier
//            std::ifstream ifs(archive_filename.c_str());
//            boost::archive::text_iarchive input_arch(ifs);
//
//            input_arch >> p_modifier2;
//
//            // See whether we read out the correct variable name
//            std::string variable_name = (static_cast<ParabolicBoxDomainPdeModifier<2>*>(p_modifier2))->mpPdeAndBcs->rGetDependentVariableName();
//            // Test that member variables are initialised correctly
//            TS_ASSERT_EQUALS(variable_name, "averaged quantity");
//
//            ///\todo #2687 The archive created a new PDE object. Memory-management of mpPdeAndBcs is not enabled. Suggest using a shared-pointer.
//            delete (static_cast<ParabolicBoxDomainPdeModifier<2>*>(p_modifier2))->mpPdeAndBcs;
//            delete p_modifier2;
//        }
//    }
};

#endif /*TESTBOXDOMAINPDEMODIFIERS_HPP_*/
