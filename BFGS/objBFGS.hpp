#ifndef BFGS_OP
#define BFGS_OP

struct BFGSSettings: OptimiserSettings{};

class BFGS: public BaseOptimiser{
public:
  double st_q, qt_D_q;
  const double max_bound = 512;
  int dim;
  Result minimise(ObjectiveFunction& f, const Eigen::VectorXd& int_val, const BFGSSettings settings)
  {
    Result res;
    // Initialise vectors to hold xk, xk+1 and print initial value.
    dim = int_val.size();
    Eigen::VectorXd next_step(dim), df(dim), next_df(dim);
    Eigen::VectorXd step = int_val;

    // Useful constants for updates
    Eigen::VectorXd s(dim), q(dim), w(dim);
    // Initialise relative changes to large number
    res.rel_sol_change = 2*settings.rel_sol_change_tol;
    res.grad_norm = 2*settings.grad_norm_tol;

    // Initialise matrix D as the identity matrix
    Eigen::MatrixXd D = Eigen::MatrixXd::Identity(dim, dim);
    Eigen::VectorXd p(dim);

    // Compute initial gradient
    df = f.gradient(step);
    // While max iteraitons has not been reached and all change tolerances have
    // not been met, perform bfgs update
    const double c =1e-4, tau = 0.95;
    while (res.iterations < settings.max_iter &&
           res.rel_sol_change > settings.rel_sol_change_tol &&
           res.grad_norm > settings.grad_norm_tol)
      {
        p= -D*df;
        // Armijo condition
        double gamma = 2;
        while(f.evaluate(step + gamma*p) > (f.evaluate(step) + gamma*c*p.transpose()*df)){
          gamma *= tau;
        }
        // Update step
        next_step = step + gamma*p;
        // Project iteration back into hypercube if needed
        for (int i=0; i<dim; ++i){
          if (next_step(i) < -max_bound){next_step(i) = -max_bound;}
          else if (next_step(i)>max_bound){next_step(i)=max_bound;}
        }

        // Compute new gradient and relative changes
        next_df = f.gradient(next_step);
        res.rel_sol_change = abs((f.evaluate(next_step) - f.evaluate(step))/f.evaluate(step));
        res.grad_norm = df.norm();


        // Update the matrix D
        // Compute difference between points and gradients at k and k+1.
        s = next_step - step;
        q = next_df - df;
        st_q = s.transpose()*q;
        qt_D_q = abs(q.transpose()*D*q); // Small values may cause erratic negative values
        w = sqrt(qt_D_q)*(s/st_q - D*q/qt_D_q);
        D += s*s.transpose()/st_q - D*q*q.transpose()*D/qt_D_q + w*w.transpose();
        // Update iterate values
        // step = next_step;
        df = next_df;
        res.iterations++;
      }
    res.minimiser = step;
    res.minimum = f.evaluate(step);
    return res;
  }
};
#endif