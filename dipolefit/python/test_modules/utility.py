import math

def ensure_distance(a, b, eps, message="Test FAILED"):
    if math.fabs(a-b) > eps:
        print(message)
        assert()
