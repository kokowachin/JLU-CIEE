clc;clear;close all;
Fs = 5000;
ts = 1/Fs;
t = 0:ts:4-ts;
x = 10*cos(200*pi*t)+8*cos(600*pi*t);

fp = 150;
fs = 200;
wp = 2*fp*pi/Fs;
ws = 2*fs*pi/Fs;
Bt = abs(wp-ws);
N0 = ceil(6.2*pi/Bt);
N1 = N0+mod(N0+1,2);
wc = (wp+ws)/2/pi;
hn = fir1(N1-1,wc,'high',hanning(N1));

N=length(t);
f1 = (-N/2:N/2-1)*Fs/N;
xt = fft(x)/N;

figure(1);
subplot(2,1,1);
plot(t,x);
xlim([0,0.1]);
subplot(2,1,2);
stem(f1,fftshift(abs(xt)));

figure(2);
freqz(hn);

y = filter(hn,1,x);
yf = fft(y)/N;

figure(3);
subplot(2,1,1);
plot(t,y);
xlim([0,0.4]);
xlabel('t');
ylabel('y(t)');
subplot(2,1,2);
stem(f1,fftshift(abs(yf)));
xlabel('频率f');
ylabel('|Y(f)|');
