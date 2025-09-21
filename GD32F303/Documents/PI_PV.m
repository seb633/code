function PI_PV(Kp,Ki)

Cpv = 680e-6;
K = 0.002;
 
s = tf('s');
Fs = 1000;
Ts = 1/Fs;

fprintf("K=%f Cpv=%f Fs=%.0f Kp=%f Ki=%f\n",K,Cpv,Fs,Kp,Ki);

Gdelay = exp(-s*Ts);
Gps = Gdelay * 1 / (Cpv*s + K);
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