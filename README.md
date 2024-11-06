# OpenGL-Animated-Robot
GLUT OpenGL C++

An animtated 3D robot with functional joints and walking.

To compile:
	- Used OpenGL on Visual Studio compilied on Windows 10 following the 'CompilingOnWindows' guide on D2L

Robot Assembly:
	Head, Body (cannon/gun included), Shoulder, Left & Right Arm, Lower Body, Left & Right Leg
	Used nested glPushMatrix() and glPopMatrix() to form hierarchial design

Controls:
	non animated movements:
		w,a,s,d -- tap to rotate or move fwd/bwd
	animated movements:
		i,j,k,l -- i & k are move fwd/bwd and automatically starts the walking animation
		g/G 	-- chest cannon movement and to stop
		'.'     -- stops all movement and resets leg position
	joint movements:
		z/Z -- shoulder (both arms)
		x/X -- elbow (both arms)
		c/C -- left leg
		f/F -- right leg
		v/V -- left knee
		b/B -- right knee

