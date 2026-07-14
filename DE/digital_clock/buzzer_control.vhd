library ieee;
use ieee.std_logic_1164.all;

entity buzzer_control is
    port (
        -- 时钟源 (来自第一步的分频模块)
        clk_1khz      : in  std_logic;  -- 1000Hz 频率 (高音/闹铃)
        clk_500hz     : in  std_logic;  -- 500Hz 频率 (低音)
        
        -- 时间和状态信息 (来自第四步的计时模块)
        time_min      : in  integer range 0 to 59;
        time_sec      : in  integer range 0 to 59;
        alarm_ringing : in  std_logic;  -- 闹铃正在响的标志
        
        -- 蜂鸣器物理引脚输出
        beep_out      : out std_logic
    );
end buzzer_control;

architecture rtl of buzzer_control is
    signal chime_en : std_logic := '0';
    signal tone_out : std_logic := '0';
begin

    -- 纯组合逻辑：判断当前时间是否满足“整点报时”的条件
    process(time_min, time_sec, clk_1khz, clk_500hz)
    begin
        -- 1. 前5次低音提示 (59分 的 50, 52, 54, 56, 58秒)
        if time_min = 59 and (time_sec = 50 or time_sec = 52 or time_sec = 54 or time_sec = 56 or time_sec = 58) then
            chime_en <= '1';
            tone_out <= clk_500hz;
            
        -- 2. 整点高音提示 (刚好 00分 00秒 时)
        elsif time_min = 0 and time_sec = 0 then
            chime_en <= '1';
            tone_out <= clk_1khz;
            
        -- 3. 其他时间不发声
        else
            chime_en <= '0';
            tone_out <= '0';
        end if;
    end process;

    -- 综合输出控制 (多路选择)
    -- 优先级：闹钟 > 整点报时 > 静音
    -- 注意：题目说“闹铃一直响”，这里直接输出 1kHz 的方波作为刺耳的连续闹铃声
    beep_out <= clk_1khz when alarm_ringing = '1' else
                tone_out when chime_en = '1' else
                '0';

end rtl;