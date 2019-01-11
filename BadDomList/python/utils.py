
def is_ice_top(omkey):
    """
    Determines if the omkey is an IceTop key or not.

    Args:
        omkey (icetray.OMKey): The OMKey

    Returns:
        bool: `True` if the omkey is an IceTop key.
    """
    if omkey.string < 1 or omkey.string > 86:
        return False
    else:
        return omkey.om >= 61 and omkey.om <= 64

