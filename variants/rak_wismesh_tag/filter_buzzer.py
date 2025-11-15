Import("env")

# Check if PIN_BUZZER is defined in CPPDEFINES
cppdefines = env.get("CPPDEFINES", [])
pin_buzzer_defined = False

for item in cppdefines:
    if isinstance(item, tuple):
        if item[0] == "PIN_BUZZER":
            pin_buzzer_defined = True
            break
    elif item == "PIN_BUZZER":
        pin_buzzer_defined = True
        break

# If PIN_BUZZER is not defined (was undefined via -U flag), exclude buzzer.cpp from build
if not pin_buzzer_defined:
    env.Append(
        BUILD_SRC_FILTER=["-<helpers/ui/buzzer.cpp>"]
    )

