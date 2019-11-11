import logging

import numpy as np
from netCDF4 import stringtochar

from refractor import framework as rf
from .base import OutputBase

logger = logging.getLogger(__name__)

class SolverIterationOutput(rf.ObserverIterativeSolver, OutputBase):
    "Notifies output classes the current retrieval iteration number when it changes"

    def __init__(self, output, step_index):
        # Required to initialize director
        rf.ObserverIterativeSolver.__init__(self)

        self.output = output
        self.step_index = step_index

    def notify_update(self, solver):
        iter_index = solver.num_accepted_steps
        base_group_name = self.iter_step_group_name(self.step_index, iter_index)

        status_dim = "status_string_s{}".format(self.step_index+1)
        if status_dim not in self.output.dimensions:
            self.output.createDimension(status_dim)

        # Solver output
        solver_group_name = base_group_name + "Solver"
        solver_group = self.output.createGroup(solver_group_name)

        logger.debug("Storing solver information into output file at {}".format(solver_group_name))
 
        if "cost_function" in solver_group.variables:
            cost_function = solver_group["cost_function"]
        else:
            cost_function = solver_group.createVariable("cost_function", float)
        cost_function[...] = solver.cost_at_accepted_points[iter_index]

        if "status" in solver_group.variables:
            status = solver_group["status"]
        else:
            status = solver_group.createVariable("status", "S1", (status_dim))
        status[:] = stringtochar(np.array(solver.status_str, "S3"))

        if "num_accepted" in solver_group.variables:
            num_accepted = solver_group["num_accepted"]
        else:
            num_accepted = solver_group.createVariable("iteration_index", int)
        num_accepted[...] = solver.num_accepted_steps

        if hasattr(solver, "problem") and hasattr(solver.problem, "max_a_posteriori"):
            apost_cov = solver.problem.max_a_posteriori.a_posteriori_covariance

            # This should be the same as in StateVectorOutput, since they are sized the same
            sv_dim = "state_vector_s{}".format(self.step_index+1)
            if sv_dim not in self.output.dimensions:
                self.output.createDimension(sv_dim, apost_cov.shape[0])

            if "covariance_a_posteriori" in solver_group.variables:
                cov = solver_group["covariance_a_posteriori"]
            else:
                cov = solver_group.createVariable("covariance_a_posteriori", float, (sv_dim, sv_dim))
            cov[:, :] = apost_cov