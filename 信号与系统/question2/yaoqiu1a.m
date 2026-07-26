clc;clear;close all;
wc = pi/4;
N1 = 15;
n1 = -(N1-1)/2:(N1-1)/2;
N2 = 33;
n2 = -(N2-1)/2:(N2-1)/2;
w = wc/pi;

h1 = fir1(N1-1,w,'low',hanning(N1));
h2 = fir1(N2-1,w,'low',hanning(N2));

H1 = fft(h1);
f1 = (-(N1-1)/2:(N1-1)/2)/N1;

H2 = fft(h2);
f2 = (-(N2-1)/2:(N2-1)/2)/N2;

figure();
subplot(3,1,1);
stem(n1,h1);
title('h1[n]的波形,N=15');
xlabel('n');
ylabel('h[n]');
subplot(3,1,2);
plot(f1,fftshift(abs(H1)));
xlabel('频率f/Hz');
ylabel('|X(f)|');
subplot(3,1,3);
plot(f1,fftshift(angle(H1)));
xlabel('频率f/Hz');
ylabel('|∠X(f)|');

figure();
freqz(h1,1);
title('N=15的频率响应图');

figure();
subplot(3,1,1);
stem(n2,h2);
title('h2[n]的波形,N=33');
xlabel('n');
ylabel('h[n]');
subplot(3,1,2);
plot(f2,fftshift(abs(H2)));
xlabel('频率f/Hz');
ylabel('|X(f)|');
subplot(3,1,3);
plot(f2,fftshift(angle(H2)));
xlabel('频率f/Hz');
ylabel('|∠X(f)|');

figure();
freqz(h2,1);
title('N=33的频率响应图');