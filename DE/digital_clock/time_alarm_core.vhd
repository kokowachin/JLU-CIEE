library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity time_alarm_core is
    port (
        clk_1khz      : in  std_logic;  -- 系统同步主时钟
        clk_1hz       : in  std_logic;  -- 1Hz 分频方波 (用于走时)

        -- 控制脉冲 (来自状态机，高电平有效且仅1个时钟周期)
        add_t_hr      : in  std_logic;  -- 调时: 小时+1
        add_t_min     : in  std_logic;  -- 调时: 分钟+1
        add_a_hr      : in  std_logic;  -- 调闹钟: 小时+1
        add_a_min     : in  std_logic;  -- 调闹钟: 分钟+1
        toggle_12_24  : in  std_logic;  -- 切换12/24小时制
        stop_alarm    : in  std_logic;  -- 停止闹钟

        -- 数据输出 (输出给显示模块)
        time_hr       : out integer range 0 to 23;
        time_min      : out integer range 0 to 59;
        time_sec      : out integer range 0 to 59;
        alarm_hr      : out integer range 0 to 23;
        alarm_min     : out integer range 0 to 59;

        -- 状态标志输出
        is_12hr_fmt   : out std_logic;  -- '1'为12小时制, '0'为24小时制
        is_pm         : out std_logic;  -- '1'表示下午(PM)
        alarm_ringing : out std_logic   -- '1'表示闹钟正在响铃
    );
end time_alarm_core;

architecture rtl of time_alarm_core is
    -- 内部时间寄存器 (默认初始时间为 12:00:00)
    signal t_hr   : integer range 0 to 23 := 12;
    signal t_min  : integer range 0 to 59 := 0;
    signal t_sec  : integer range 0 to 59 := 0;

    -- 内部闹钟寄存器 (默认初始闹钟为 12:01)
    signal a_hr   : integer range 0 to 23 := 12;
    signal a_min  : integer range 0 to 59 := 1;

    -- 1Hz 边沿检测信号
    signal clk_1hz_d1 : std_logic := '0';
    signal clk_1hz_d2 : std_logic := '0';
    signal sec_pulse  : std_logic := '0';

    -- 标志位寄存器
    signal fmt_12   : std_logic := '0'; -- 默认24小时制
    signal ringing  : std_logic := '0'; -- 默认不响铃

begin

    -- 边沿检测逻辑：将 50% 占空比的 1Hz 方波转换为只有 1 个 clk_1khz 周期的短脉冲
    process(clk_1khz)
    begin
        if rising_edge(clk_1khz) then
            clk_1hz_d1 <= clk_1hz;
            clk_1hz_d2 <= clk_1hz_d1;
        end if;
    end process;
    -- 当上一拍是1，上上拍是0时，说明检测到了上升沿
    sec_pulse <= '1' when (clk_1hz_d1 = '1' and clk_1hz_d2 = '0') else '0';


    -- 核心逻辑进程
    process(clk_1khz)
    begin
        if rising_edge(clk_1khz) then
            
            -------------------------------------------------
            -- 1. 时间和闹钟的按键设置逻辑 (优先级高于自然走时)
            -------------------------------------------------
            if add_t_hr = '1' then
                if t_hr = 23 then t_hr <= 0; else t_hr <= t_hr + 1; end if;
            elsif add_t_min = '1' then
                if t_min = 59 then t_min <= 0; else t_min <= t_min + 1; end if;
                t_sec <= 0; -- 按照手表惯例，调分时秒钟自动归零

            elsif add_a_hr = '1' then
                if a_hr = 23 then a_hr <= 0; else a_hr <= a_hr + 1; end if;
            elsif add_a_min = '1' then
                if a_min = 59 then a_min <= 0; else a_min <= a_min + 1; end if;

            -------------------------------------------------
            -- 2. 正常走时逻辑 (依赖秒脉冲 sec_pulse)
            -------------------------------------------------
            elsif sec_pulse = '1' then
                if t_sec = 59 then
                    t_sec <= 0;
                    if t_min = 59 then
                        t_min <= 0;
                        if t_hr = 23 then
                            t_hr <= 0;
                        else
                            t_hr <= t_hr + 1;
                        end if;
                    else
                        t_min <= t_min + 1;
                    end if;
                else
                    t_sec <= t_sec + 1;
                end if;
            end if;

            -------------------------------------------------
            -- 3. 其他状态切换与闹铃逻辑
            -------------------------------------------------
            -- 12/24进制切换
            if toggle_12_24 = '1' then
                fmt_12 <= not fmt_12;
            end if;

            -- 闹铃触发判断：时间匹配，且刚好是这一秒的起点
            if t_hr = a_hr and t_min = a_min and t_sec = 0 and sec_pulse = '1' then
                ringing <= '1';
            end if;

            -- 关闹铃逻辑
            if stop_alarm = '1' then
                ringing <= '0';
            end if;

        end if;
    end process;

    -- 将内部寄存器映射到输出端口
    time_hr  <= t_hr;
    time_min <= t_min;
    time_sec <= t_sec;
    alarm_hr <= a_hr;
    alarm_min<= a_min;

    is_12hr_fmt   <= fmt_12;
    alarm_ringing <= ringing;
    
    -- 下午(PM)判断：当小时大于等于12时即为下午
    is_pm <= '1' when t_hr >= 12 else '0';

end rtl;