library ieee;
use ieee.std_logic_1164.all;

entity digital_clock is
    port (
        sys_clk   : in  std_logic;                     -- 硬件引脚：2.048MHz 晶振输入
        key_in    : in  std_logic_vector(1 downto 0);  -- 硬件引脚：2个独立按键 (默认高电平，按下低电平)
        
        seg_out   : out std_logic_vector(7 downto 0);  -- 硬件引脚：数码管段选 (DP,G,F,E,D,C,B,A)
        sel_out   : out std_logic_vector(7 downto 0);  -- 硬件引脚：数码管位选 (DIG7 ~ DIG0)
        beep_out  : out std_logic;                      -- 硬件引脚：蜂鸣器控制输出
		  led_red   : out std_logic;
        led_yellow: out std_logic
    );
end digital_clock;

architecture structural of digital_clock is

    ---------------------------------------------------------------------------
    -- 1. 声明所有子模块 (Component Declarations)
    -- 就像列出装配清单，告诉 Quartus 我们需要用到哪些零件
    ---------------------------------------------------------------------------
    
    component clk_divider is
        port (
            clk_in    : in  std_logic;
            clk_1hz   : out std_logic;
            clk_1khz  : out std_logic;
            clk_500hz : out std_logic
        );
    end component;

    component key_debounce is
        port (
            clk_1khz  : in  std_logic;
            key_in    : in  std_logic_vector(1 downto 0);
            key_pulse : out std_logic_vector(1 downto 0)
        );
    end component;

    component state_machine is
        port (
            clk_1khz      : in  std_logic;
            key_pulse     : in  std_logic_vector(1 downto 0);
            alarm_ringing : in  std_logic;
            state_out     : out integer range 0 to 5;
            add_t_hr      : out std_logic;
            add_t_min     : out std_logic;
            add_a_hr      : out std_logic;
            add_a_min     : out std_logic;
            toggle_12_24  : out std_logic;
            stop_alarm    : out std_logic
        );
    end component;

    component time_alarm_core is
        port (
            clk_1khz      : in  std_logic;
            clk_1hz       : in  std_logic;
            add_t_hr      : in  std_logic;
            add_t_min     : in  std_logic;
            add_a_hr      : in  std_logic;
            add_a_min     : in  std_logic;
            toggle_12_24  : in  std_logic;
            stop_alarm    : in  std_logic;
            time_hr       : out integer range 0 to 23;
            time_min      : out integer range 0 to 59;
            time_sec      : out integer range 0 to 59;
            alarm_hr      : out integer range 0 to 23;
            alarm_min     : out integer range 0 to 59;
            is_12hr_fmt   : out std_logic;
            is_pm         : out std_logic;
            alarm_ringing : out std_logic
        );
    end component;

    component buzzer_control is
        port (
            clk_1khz      : in  std_logic;
            clk_500hz     : in  std_logic;
            time_min      : in  integer range 0 to 59;
            time_sec      : in  integer range 0 to 59;
            alarm_ringing : in  std_logic;
            beep_out      : out std_logic
        );
    end component;

    component display_driver is
        port (
            clk_1khz    : in  std_logic;
            clk_1hz     : in  std_logic;
            state       : in  integer range 0 to 5;
            time_hr     : in  integer range 0 to 23;
            time_min    : in  integer range 0 to 59;
            time_sec    : in  integer range 0 to 59;
            alarm_hr    : in  integer range 0 to 23;
            alarm_min   : in  integer range 0 to 59;
            is_12hr_fmt : in  std_logic;
            is_pm       : in  std_logic;
            seg_out     : out std_logic_vector(7 downto 0);
            sel_out     : out std_logic_vector(7 downto 0)
        );
    end component;
	 
	 component led_control is
        port (
            is_12hr_fmt : in  std_logic;
            is_pm       : in  std_logic;
            led_red     : out std_logic;
            led_yellow  : out std_logic
        );
    end component;
    ---------------------------------------------------------------------------
    -- 2. 声明内部信号线 (Signal Declarations)
    -- 相当于主板上的铜线，用于把各个模块的引脚连接起来
    ---------------------------------------------------------------------------
    
    -- 时钟信号线
    signal sig_clk_1hz   : std_logic;
    signal sig_clk_1khz  : std_logic;
    signal sig_clk_500hz : std_logic;
    
    -- 按键与控制信号线
    signal sig_key_pulse : std_logic_vector(1 downto 0);
    signal sig_state     : integer range 0 to 5;
    
    signal sig_add_t_hr  : std_logic;
    signal sig_add_t_min : std_logic;
    signal sig_add_a_hr  : std_logic;
    signal sig_add_a_min : std_logic;
    signal sig_toggle_12_24 : std_logic;
    signal sig_stop_alarm   : std_logic;
    
    -- 时间与状态数据线
    signal sig_time_hr   : integer range 0 to 23;
    signal sig_time_min  : integer range 0 to 59;
    signal sig_time_sec  : integer range 0 to 59;
    signal sig_alarm_hr  : integer range 0 to 23;
    signal sig_alarm_min : integer range 0 to 59;
    
    signal sig_is_12hr_fmt : std_logic;
    signal sig_is_pm       : std_logic;
    signal sig_alarm_ringing : std_logic;

begin

    ---------------------------------------------------------------------------
    -- 3. 实例化与端口映射 (Port Mapping)
    -- 将“内部信号线”或“外部引脚”连接到“零件”上
    ---------------------------------------------------------------------------

    -- 1. 实例化分频器
    u1_clk_divider: clk_divider port map (
        clk_in    => sys_clk,
        clk_1hz   => sig_clk_1hz,
        clk_1khz  => sig_clk_1khz,
        clk_500hz => sig_clk_500hz
    );

    -- 2. 实例化按键消抖
    u2_key_debounce: key_debounce port map (
        clk_1khz  => sig_clk_1khz,
        key_in    => key_in,
        key_pulse => sig_key_pulse
    );

    -- 3. 实例化状态机
    u3_state_machine: state_machine port map (
        clk_1khz      => sig_clk_1khz,
        key_pulse     => sig_key_pulse,
        alarm_ringing => sig_alarm_ringing,
        state_out     => sig_state,
        add_t_hr      => sig_add_t_hr,
        add_t_min     => sig_add_t_min,
        add_a_hr      => sig_add_a_hr,
        add_a_min     => sig_add_a_min,
        toggle_12_24  => sig_toggle_12_24,
        stop_alarm    => sig_stop_alarm
    );

    -- 4. 实例化时间与闹钟核心
    u4_time_alarm_core: time_alarm_core port map (
        clk_1khz      => sig_clk_1khz,
        clk_1hz       => sig_clk_1hz,
        add_t_hr      => sig_add_t_hr,
        add_t_min     => sig_add_t_min,
        add_a_hr      => sig_add_a_hr,
        add_a_min     => sig_add_a_min,
        toggle_12_24  => sig_toggle_12_24,
        stop_alarm    => sig_stop_alarm,
        time_hr       => sig_time_hr,
        time_min      => sig_time_min,
        time_sec      => sig_time_sec,
        alarm_hr      => sig_alarm_hr,
        alarm_min     => sig_alarm_min,
        is_12hr_fmt   => sig_is_12hr_fmt,
        is_pm         => sig_is_pm,
        alarm_ringing => sig_alarm_ringing
    );

    -- 5. 实例化蜂鸣器控制
    u5_buzzer_control: buzzer_control port map (
        clk_1khz      => sig_clk_1khz,
        clk_500hz     => sig_clk_500hz,
        time_min      => sig_time_min,
        time_sec      => sig_time_sec,
        alarm_ringing => sig_alarm_ringing,
        beep_out      => beep_out
    );

    -- 6. 实例化数码管显示驱动
    u6_display_driver: display_driver port map (
        clk_1khz    => sig_clk_1khz,
        clk_1hz     => sig_clk_1hz,
        state       => sig_state,
        time_hr     => sig_time_hr,
        time_min    => sig_time_min,
        time_sec    => sig_time_sec,
        alarm_hr    => sig_alarm_hr,
        alarm_min   => sig_alarm_min,
        is_12hr_fmt => sig_is_12hr_fmt,
        is_pm       => sig_is_pm,
        seg_out     => seg_out,
        sel_out     => sel_out
    );
-- 7. 实例化 LED 控制
    u7_led_control: led_control port map (
        is_12hr_fmt => sig_is_12hr_fmt,  -- 连到内部判断 12/24 制的信号线上
        is_pm       => sig_is_pm,        -- 连到内部判断 PM 的信号线上
        led_red     => led_red,          -- 连到外部物理红灯引脚
        led_yellow  => led_yellow        -- 连到外部物理黄灯引脚
    );
end structural;