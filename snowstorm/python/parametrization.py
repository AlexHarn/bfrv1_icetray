class Parametrization(object):
    """
    Base class for all parametrizations.
    """
    def transform(self, x, frame):
        """
        Transform any objects in `frame` according to the parameters `x`.

        :param x: The parameter vector for the transformation.

        :param frame: The frame holding the objects for the transformation.
        """
        raise NotImplementedError