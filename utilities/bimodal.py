from dl_utilities import bracket_search, trapezoidal_rule

from datetime import datetime
import copy
import numpy as np
import random
from math import sqrt

class BimodalFun:
  """
  Class to manage a two modal distribution.

  Members
  -------

  _params: dict
    A dictionary containing all the parameters needed for
    generating the bimodal function.

  Methods
  -------

  take_params
    Function receiving the parameters and invoking the
    generation of the new function.

  eval
    Function to evaluate the function in one or more points.

  _generate_curve
    Function to generate a curve given the parameters
    stored inside the class.

  """



  def __init__(self, params):
    """
    Parameters
    ----------

    self: pointer
      Object pointer

    params: dict
      Dictionary with all the parameters.
      See method take_params().

    """
    self.take_params(params)
    return



  def take_params(self, params):
    """
    Method to take the input parameters and (re)generate
    the function according to the new parameters.

    Parameters
    ----------

    self: pointer
      Object pointer

    params: dict
      Dictionary with all the needed parameters.
      They are:
      "vmin": float, minimum value of the function.
      "vmax": float, maximum value of the function.
      "tmin": int, initial time (independent variable).
      "tmax": int, final time.
      "a": float, parameter at the exponent of the first
      exponential.
      "b": float, parameter at the exponent of the second
      exponential.
      "alpha": floar, ratio between first peak and second
      peak heights.
      "seed": int, seed for the random generation of the peaks.
      "peaks": list, the position of the two peaks.
      Giving [] as input activates the random generation of the
      peaks (see seed).

    """
    self._params = params
    if bool(self._params):
      self._curve = self._generate_curve()

    return



  def eval(self, t, smoother = False):
    """
    Function to evaluate the function in a single point
    or in an array of points.

    Parameters
    ----------

    self: pointer
      Object pointer

    t: float / numpy array
      The point / array of points containing the times
      i.e., the independent variable.

    smoother: bool (default = False)
      True if the evaluation must be executed using the average values
      of the intervals, usually offering a smoother result.

    Return
    ------

    self._curve(t): float / numpy array
      The values coming from the evaluation of the function.

    """
    # Normal function evaluation
    if not smoother:
      return self._curve(t)
    # Evaluation via average value
    else:

      # For ease
      tmin = self._params["tmin"]

      # Later use
      one_less = False

      # If dealing with numpy arrays
      if isinstance(t, np.ndarray):

        # Check if the initial point is not already inside t
        if abs(tmin - t[0]) > 1e-6:

          # Create an array with the left points, including tmin
          #   and leaving out the last element
          left_points = np.array([tmin])
          left_points = np.append(left_points, t[0:t.size - 1])

        else:

          # In this case, the initial point is not added
          left_points = t[0:t.size - 1]
          t = t[1:]

          # With this procedure one less point will be computed now
          one_less = True

      else:

        # Check if the initial point is the given one
        if abs(tmin - t) < 1e-6:
          raise RuntimeError("When giving just one point for the smooth " \
            "evaluation, it must be different from tmin.")

        # The left point is the first one
        left_points = tmin

      # Compute the interval lengths
      lengths = t - left_points

      # Compute the integrals
      integrals = trapezoidal_rule(fun = self._curve, t1 = left_points,
        t2 = t, n_intervals = 1000)

      # Compute the values (adding the pointwise first when missing one)
      if not one_less:
        return integrals / lengths
      else:
        return np.append(self._curve(tmin), integrals / lengths)



  def _generate_curve(self):
    """
    Method to generate the bimodal curve.
    The function has the following form:
      v(t) = vmin +
        (vmax - vmin) * (
          exp(-a * (t - t_peak_1)**2) +
          alpha * exp(-b * (t - t_peak_2)**2)
        ) / C
    where C is only a normalisation term.

    Parameters
    ----------

    self: pointer
      Object pointer

    Returns
    -------

    curve: function
      The bimodal function, mapping:
      float -> float
      np.array -> np.array.

    """
    # For ease of use
    seed = self._params["seed"]
    peaks = self._params["peaks"]
    vmin = self._params["vmin"]
    vmax = self._params["vmax"]
    tmin = self._params["tmin"]
    tmax = self._params["tmax"]
    a = self._params["a"]
    b = self._params["b"]
    alpha = self._params["alpha"]

    # Peaks will be simulated
    if not len(peaks):

      # Fix the seed for the simulation of the peaks
      random.seed(seed)

      # Simulate the peaks
      t_peak_1 = max(tmin, \
        min(random.gauss(0.15 * (tmax + tmin), sqrt(0.1 * (tmax - tmin))), tmax))
      t_peak_2 = max(tmin, \
        min(random.gauss(0.5 * (tmax + t_peak_1), sqrt(0.1 * (tmax - tmin))), tmax))

    # Peaks are assigned from the outside
    elif len(peaks) == 2:

      # Assign the peaks values
      t_peak_1 = peaks[0]
      t_peak_2 = peaks[1]

      # Check the peaks order
      if (t_peak_1 > t_peak_2):
        raise RuntimeError("The first peak comes after the second. " \
          "Invert their order.")

    # No other case
    else:
      raise RuntimeError("Peaks must be a list with either two elements " \
        "or empty.")

    # Create the function
    # First bell
    first_exp = lambda t : np.exp(-a * (t - t_peak_1)**2)

    # Second bell
    second_exp = lambda t : np.exp(-b * (t - t_peak_2)**2)

    # Function which zero is needed for the normalisation part
    to_be_zero = lambda t : a * (t - t_peak_1) * first_exp(t) \
      + alpha * b *(t - t_peak_2) * second_exp(t)

    # Compute normalisation coefficient
    to_right = alpha < 1.
    [normalisation_point, converged] = \
      bracket_search(to_be_zero,
      t_peak_1 * to_right + t_peak_2 * (not to_right), to_right,
      (t_peak_2 - t_peak_1)/200, 1e-9, int(1e+4))[0:2]

    # Warn in case of imperfect research of the zero
    if not converged:
      raise RuntimeWarning("The research of the zero did not converge.")

    # Everything together
    curve = lambda t : vmin + (vmax - vmin) \
      * (first_exp(t) + alpha * second_exp(t)) \
      / (first_exp(normalisation_point) + alpha * second_exp(normalisation_point))

    return curve



#####################
#####################
#####################



class BimodalEnsemble:
  """
  Class to ensemble single bimodal functions to create a function
  compounding more functions one after the other.

  Members
  -------

  _params: dict
    Dictionary with the parameters of the whole final function.

  _n_bimodals: int
    The number of the single bimodal functions to be ensembled.

  Methods
  -------

  _generate()
    Generate the function composed by the bimodal functions.


  """

  def __init__(self, params, n_bimodals = 1):
    """
    Parameters
    ----------

    self
      The object pointer

    params: dict
      Dictionary with the parameters for the bimodals.
      They are:
      "vmin": float, minimum value of the function.
      "vmax": float, maximum value of the function.
      "tmin": int, initial time (independent variable).
      "tmax": int, final time.
      "seed": int, seed for the random generation of the peaks.

    n_bimodals: int
      The number of bimodal functions to be fit inside the interval.

    """
    self._params = params
    self._n_bimodals = n_bimodals
    self._generate()



  def _generate(self):
    """
    Method to generate a numpy array containing all the bimodal curves
    which compose the final object.

    """

    # Initialise the objects storage
    self._curves = np.array([BimodalFun({})] * self._n_bimodals)

    # 1/empty_space is not occupied by any curve
    empty_space = 2

    # Initialise values
    bimodal_params = copy.deepcopy(self._params)
    delta_time = (self._params["tmax"] - self._params["tmin"]) \
      / (self._n_bimodals * empty_space)
    bimodal_params["tmax"] = bimodal_params["tmin"] + delta_time

    # Check for a seed
    seeded = bool("seed" in bimodal_params)

    # Loop to create the single bimodal functions
    for i in range(0, self._n_bimodals * empty_space):

      if i % empty_space == 0:

        # Seed for random generation
        if seeded:
          if isinstance(bimodal_params["seed"], int):
            np.random.seed(bimodal_params["seed"] + i)
          else:
            np.random.seed(bimodal_params["seed"])
        else:
          bimodal_params["seed"] = int(round(datetime.now().timestamp()) + i)
          seeded = False

        # Generate the peaks
        np.random.seed(bimodal_params["seed"])
        bimodal_params["peaks"] = np.array([])

        # Generate alpha
        bimodal_params["alpha"] = np.random.uniform(0.5, 2.)

        # Generate the parameters at the exponential
        bimodal_params["a"] = np.random.uniform(7.5, 15) \
          / (bimodal_params["tmax"] - bimodal_params["tmin"])**2
        bimodal_params["b"] = np.random.uniform(10, 17.5) \
          / (bimodal_params["tmax"] - bimodal_params["tmin"])**2

        # Give the new parameters and generate the new curve
        self._curves[int(i/empty_space)] = BimodalFun(bimodal_params)

      # Update times
      bimodal_params["tmin"] += delta_time
      bimodal_params["tmax"] += delta_time



  def eval(self, t, average = False):
    """
    Method to evaluate the ensemble of functions in one or more points.

    Parameters
    ----------

    self
      The object pointer.

    t: float / numpy array
      The value(s) where the function must be evaluated.

    average: bool (default = False)
      True if the single functions must be evaluated with the average
      value instead of the single point.

    Return
    ------

    The value(s) of the ensemble evaluated in the input points.

    """

    # Initialise values
    value = 0.

    # Sum each contribution
    for single_curve in self._curves:
      value += single_curve.eval(t, average) - self._params["vmin"]
    # REMARK: vmin must be subtracted otherwise they stack after each function

    return value + self._params["vmin"]
