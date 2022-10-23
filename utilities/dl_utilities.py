import numpy as np

def bracket_search(fun, x, from_left = True, delta = 0.001,
  tol = 1e-9, max_iter = 1000):
  """
  Function to implement a bracket search, followed by a bisection method.

  Parameters
  ----------

  fun: function
    The function which zero is desired.

  x: float
    Initial point where the research must start.

  from_left: bool (default = True)
    True if the interval must grow to the right, false otherwise.

  delta: float (default = 0.001)
    The size of the increment of the interval at each iteration.
    The sign does not matter, as it is decided by from_left.

  tol: float (default = 1e-9)
    Tolerance for the proximity to the zero.

  max_iter: int (default = 1000)
    Maximum number of iterations, both for the bracket search and the
    bisection method.

  Return
  ------

  c: float
    The position of the zero.

  converged: bool
    True if the bisection method reached a solution without hitting
    the maximum iteration cap.

  i: int
    Number of iterations required by the bisection method.

  """
  
  # Variation for the bracket search
  delta = abs(delta) if from_left else -abs(delta)

  # Look for the first interval containing the zero
  value_x = fun(x)
  x_moving = x
  zero_intercepted = False

  # The interval becomes larger until a zero is found
  for i in range(0, max_iter):
    x_moving += delta
    if (value_x * fun(x_moving) <= 0.):
      zero_intercepted = True
      break
  
  if not zero_intercepted:
    raise RuntimeError("No zero could be intercepted by any interval.")
  else:
    a = min(x, x_moving)
    b = max(x, x_moving)

  # Check the sign change
  if(fun(a) * fun(b) > 0.):
    raise RuntimeError("The function must change sign at the interval extrema.")

  # To exit early
  converged = False

  # Performing iterations
  for i in range(0, max_iter):

    # Middle point
    c = (a + b) / 2

    # Check convergence
    if (abs(fun(c)) < tol):
      converged = True

    # Early exit
    if converged:
      break

    # Find which interval contains the zero
    if (fun(a) * fun(c) <= 0.):
      b = c
    else:
      a = c
  
  # Warn in case the convergence was not satisfied
  if not converged:
    raise RuntimeWarning("The algorithm did not converge in " \
      + str(max_iter) + " iterations.")
  
  return c, converged, i



def trapezoidal_rule_single(fun, t1, t2):
  """
  Function to compute the integral approximation via trapezoidal rule
  of a single interval.

  Parameters
  ----------
  
  fun: function
    Function to compute the integral for.

  t1: float / numpy array
    Initial point(s) of the interval.

  t2: float / numpy array
    Final point(s) of the interval.

  Return
  ------

  integrals: float / numpy array
    Integral(s) computed with the trapezoidal rule.

  """
  # Compute the left bases
  left_bases = fun(t1)

  # Compute the right bases
  right_bases = fun(t2)

  # Compute the interval lengths (i.e., trapezoidal heights)
  heights = t2 - t1

  # Use trapezoidal rule
  integrals = (left_bases + right_bases) / 2 * heights

  return integrals



def trapezoidal_rule(fun, t1, t2, n_intervals = 100):
  """
  Function to compute the integral of a function, using the
  trapezoidal rule for the approximation.

  Parameters
  ----------

  fun: function
    The function to compute the integral of.
    It must map either
      float -> float
    or
      numpy array -> numpy array

    t1: float / numpy array
      The left side(s) of the interval(s).

    t2: float / numpy array
      The right side(s) of the interval(s).

    n_intervals: int (default = 100)
      Number of intervals to subdivide the domain.

    Return
    ------

    integrals: float / numpy array
      The approximated integrals.

  """

  # Intervals length
  int_length = (t2 - t1) / n_intervals
  
  # Initialise integrals
  if isinstance(t1, np.ndarray):
    integrals = np.zeros(t1.shape)
  else:
    integrals = 0.

  # Loop on the number of intervals
  for _ in range(0, n_intervals):

    # Move the right boundary
    t_next = t1 + int_length

    # Compute the integral for the single interval and add to the sum
    integrals += trapezoidal_rule_single(fun, t1, t_next)

    # Move the left boundary
    t1 = t_next

  return integrals



def dl_cumsum(l):
  """
  Function to compute the cumulative sum of a list
  or a numpy array.

  Parameters
  ----------

  l: list / numpy array
    Container with the elements to be summed.

  Return
  ------

  ADD RETURN
  """

  # Length of the input list
  l_length = len(l)

  # Sum the elements cumulatively
  final_list = [sum(l[0:x]) for x in range(1, l_length + 1)]

  # Transform in a numpy array if the input is a numpy array
  if isinstance(l, np.ndarray):
    final_list = np.array(final_list)

  return final_list