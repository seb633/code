function PI_U(Kp,Ki)

R = 10000;
C = 1200e-6;
s = tf('s');
Fs = 1000;
Ts = 1/Fs;

fprintf("R=%f C=%f Fs=%.0f Kp = %f Ki=%f\r\n",R,C,Fs,Kp,Ki);

Gdelay = exp(-s*Ts);
Gps = Gdelay * R / (1+s*R*C);
Gcs = Kp + Ki/s;
Gs = Gcs*Gps;

% subplot(3,1,1);
% bode(Gcs);
% margin(Gcs);
% subplot(3,1,2);
% bode(Gps);
% margin(Gps);
% subplot(3,1,3);
bode(Gs);
margin(Gs);

end