clc;clear;close all;

%% 输入音频
[Y,Fs]=audioread('title.wav');
sound(Y,Fs);
nChannels = size(Y, 2)
pause(3);
Ts = 1/Fs;
f1 = 1.1e4;
Y1 = Y(:,1);
N = length(Y1);
t = 0:Ts:(N-1)*Ts;
n = 0.1*sin(2*pi*f1*t);
x = Y1 + n';    %加入噪声的音频

%% 对音频进行fft
if(mod(N,2)==0)
    k = -N/2:N/2-1;
else
    k= -(N-1)/2:(N-1)/2;
end;
f = k*Fs/N;
Yt = fft(Y1)*Fs;

figure();
plot(f,fftshift(abs(Yt)));
xlim([-12000,12000]);
title('原音频幅值');
xlabel('频率f');
ylabel('频域幅值|X(F)|');

xt = fft(x)*Fs;
figure();
plot(f,fftshift(abs(xt)));
xlim([-12000,12000]);
title('加入噪声后的音频');
xlabel('频率f');
ylabel('频域幅值|X(F)|');

sound(x,Fs);   %播放加入噪声的音频
pause(3);


%% 滤波
fs = 8000;
fp = 10000;
ws = 2*fs*pi/Fs;
wp = 2*fp*pi/Fs;
Bt = abs(ws-wp);
N0 = ceil(6.2*pi/Bt);
N1 = N0+mod(N0+1,2);
wc = (ws+wp)/2/pi;
hn = fir1(N1-1,wc,'low',hanning(N1));

z1 = filter(hn,1,x);
zf = fft(z1)/N;
figure(3);
plot(f,fftshift(abs(zf)));
xlim([-12000,12000]);
title('滤波后的音频')
xlabel('频率f');
ylabel('频域幅值|X(F)|');

sound(z1,Fs);