MAX31865_FAULT_HIGHTHRESH = 0x80
MAX31865_FAULT_LOWTHRESH = 0x40
MAX31865_FAULT_REFINLOW = 0x20
MAX31865_FAULT_REFINHIGH = 0x10
MAX31865_FAULT_RTDINLOW = 0x08
MAX31865_FAULT_OVUV = 0x04


def log(temp: float, fault: int) -> None:
    if fault:
        if fault & MAX31865_FAULT_HIGHTHRESH:
            print("RTD High Threshold")
        if fault & MAX31865_FAULT_LOWTHRESH:
            print("RTD Low Threshold")
        if fault & MAX31865_FAULT_REFINLOW:
            print("REFIN- > 0.85 x Bias")
        if fault & MAX31865_FAULT_REFINHIGH:
            print("REFIN- < 0.85 x Bias - FORCE- open")
        if fault & MAX31865_FAULT_RTDINLOW:
            print("RTDIN- < 0.85 x Bias - FORCE- open")
        if fault & MAX31865_FAULT_OVUV:
            print("Under/Over voltage")
    else:
        print(temp)
