function PI(Kp,Ki)

Udc = 90;
Ubat = 70;
L=50e-6;
r=0.01;
s = tf('s');
Fs = 20000;
Ts = 1/Fs;

fprintf("Udc=%f Ubat=%f L=%f r=%f Fs=%.0f Kp = %f Ki=%f\n",Udc,Ubat,L,r,Fs,Kp,Ki);

Gdelay = exp(-s*Ts);
Gps = Gdelay * (Ubat + Udc) / (L*s + r);
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