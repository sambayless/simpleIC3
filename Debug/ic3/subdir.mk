################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../ic3/Cone.cpp \
../ic3/Ternary.cpp \
../ic3/ic3Types.cpp \
../ic3/ic3config.cpp 

OBJS += \
./ic3/Cone.o \
./ic3/Ternary.o \
./ic3/ic3Types.o \
./ic3/ic3config.o 

CPP_DEPS += \
./ic3/Cone.d \
./ic3/Ternary.d \
./ic3/ic3Types.d \
./ic3/ic3config.d 


# Each subdirectory must supply rules for building sources it contributes
ic3/%.o: ../ic3/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++1y -I.././ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


