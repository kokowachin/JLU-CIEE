clc;close all;clear;
fs1 = 100;
fs2 = 10;
ts1 = 1/fs1;
ts2 = 1/fs2;
t1 = 0:ts1:100;
t2 = 0:ts2:100;
t3 = 0:ts2:5;%分辨率低

x1 = rectpuls(t1-1.5,1);
x2 = rectpuls(t2-1.5,1);
x3 = rectpuls(t3-1.5,1);
fx1 = fft(x1)*ts1;

N1 = length(t1);
if(mod(N1,2)==0)
    k1 = -N1/2:N1/2-1;
else
    k1 = -(N1-1)/2:(N1-1)/2;
end;
f1 = k1*fs1/N1;
figure();
subplot(2,1,1);
plot(t1,x1);
xlim([-1,4]);
ylim([-0.5,1.5]);
subplot(2,1,2);
plot(f1,fftshift(abs(fx1)));

N2 = length(t2);
if(mod(N2,2)==0)
    k2 = -N2/2:N2/2-1;
else
    k2 = -(N2-1)/2:(N2-1)/2;
end;
f2 = k2*fs2/N2;
fx2 = fft(x2)*ts2;
figure();
subplot(2,1,1);
plot(t2,x2);
xlim([-1,4]);
ylim([-0.5,1.5]);
subplot(2,1,2);
plot(f2,fftshift(abs(fx2)));

F = sin(pi*f1)./(pi*f1);
F(f1==0)=1;
figure();
plot(f1,fftshift(abs(fx1)),'r');
hold on;
plot(f2,fftshift(abs(fx2)),'b');
plot(f1,abs(F),'g');
xlim([-10,10]);
hold off;

N3 = length(t3);
if(mod(N3,2)==0)
    k3 = -N3/2:N3/2-1;
else
    k3 = -(N3-1)/2:(N3-1)/2;
end;
f3 = k3 * fs2/N3;
ft3 = fft(x3)*ts2;
figure()
subplot(2,1,1);
plot(t3,x3);
xlim([-1,4]);
ylim([-0.5,1.5]);
subplot(2,1,2);
plot(f3,fftshift(abs(ft3)))