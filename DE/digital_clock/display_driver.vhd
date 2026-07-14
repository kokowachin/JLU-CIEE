library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity display_driver is
    port (
        clk_1khz    : in  std_logic;  -- 1kHz 用于动态扫描
        clk_1hz     : in  std_logic;  -- 1Hz 用于闪烁掩码
        
        -- 数据输入
        state       : in  integer range 0 to 5;
        time_hr     : in  integer range 0 to 23;
        time_min    : in  integer range 0 to 59;
        time_sec    : in  integer range 0 to 59;
        alarm_hr    : in  integer range 0 to 23;
        alarm_min   : in  integer range 0 to 59;
        
        is_12hr_fmt : in  std_logic;
        is_pm       : in  std_logic;
        
        -- 数码管物理引脚输出 (共阳极)
        seg_out     : out std_logic_vector(7 downto 0); -- 段选: DP,G,F,E,D,C,B,A (低电平亮)
        sel_out     : out std_logic_vector(7 downto 0)  -- 位选: 7到0 (假设低电平有效，三极管驱动)
    );
end display_driver;

architecture rtl of display_driver is

    signal scan_cnt : integer range 0 to 7 := 0;
    
    -- 准备显示的数据寄存器
    signal disp_hr  : integer range 0 to 23;
    signal disp_min : integer range 0 to 59;
    signal disp_sec : integer range 0 to 60;
    
    -- 拆分后的十位和个位 (0-9)
    -- 拆分后的十位和个位 (上限放宽至 15，以兼容 10分隔符 和 11空白符)
    signal hr_t, hr_u   : integer range 0 to 15;
    signal min_t, min_u : integer range 0 to 15;
    signal sec_t, sec_u : integer range 0 to 15;
    
    -- 当前扫描位需要显示的单个数字 (0-15: 0-9为数字, 10为横杠-, 11为空白)
    signal current_num : integer range 0 to 15;
    
    -- 闪烁控制标志
    signal blink_en : std_logic;

begin

    ---------------------------------------------------------
    -- 1. 数据源选择与 12/24 小时制转换
    ---------------------------------------------------------
    process(state, time_hr, time_min, time_sec, alarm_hr, alarm_min, is_12hr_fmt)
        variable temp_hr : integer range 0 to 23;
    begin
        -- 判断当前看的是走时还是闹钟
        if state = 3 or state = 4 then
            temp_hr  := alarm_hr;
            disp_min <= alarm_min;
            disp_sec <= 60; -- 用 60 表示全灭/不显示秒
        else
            temp_hr  := time_hr;
            disp_min <= time_min;
            disp_sec <= time_sec;
        end if;
        
        -- 12/24制转换
        if is_12hr_fmt = '1' then
            if temp_hr = 0 then
                disp_hr <= 12;
            elsif temp_hr > 12 then
                disp_hr <= temp_hr - 12;
            else
                disp_hr <= temp_hr;
            end if;
        else
            disp_hr <= temp_hr;
        end if;
    end process;

    ---------------------------------------------------------
    -- 2. 节省资源的十位/个位拆分 (Bin to BCD)
    ---------------------------------------------------------
    process(disp_hr, disp_min, disp_sec)
        variable t : integer range 0 to 9;
        variable u : integer range 0 to 59;
    begin
        -- 算小时
        u := disp_hr; t := 0;
        if u >= 20 then t := 2; u := u - 20;
        elsif u >= 10 then t := 1; u := u - 10;
        end if;
        hr_t <= t; hr_u <= u;
        
        -- 算分钟
        u := disp_min; t := 0;
        if u >= 50 then t := 5; u := u - 50;
        elsif u >= 40 then t := 4; u := u - 40;
        elsif u >= 30 then t := 3; u := u - 30;
        elsif u >= 20 then t := 2; u := u - 20;
        elsif u >= 10 then t := 1; u := u - 10;
        end if;
        min_t <= t; min_u <= u;

        -- 算秒钟
        if disp_sec = 60 then
            sec_t <= 11; sec_u <= 11; -- 11代表空白
        else
            u := disp_sec; t := 0;
            if u >= 50 then t := 5; u := u - 50;
            elsif u >= 40 then t := 4; u := u - 40;
            elsif u >= 30 then t := 3; u := u - 30;
            elsif u >= 20 then t := 2; u := u - 20;
            elsif u >= 10 then t := 1; u := u - 10;
            end if;
            sec_t <= t; sec_u <= u;
        end if;
    end process;

    ---------------------------------------------------------
    -- 3. 动态扫描与数据多路复用
    ---------------------------------------------------------
    process(clk_1khz)
    begin
        if rising_edge(clk_1khz) then
            if scan_cnt = 7 then
                scan_cnt <= 0;
            else
                scan_cnt <= scan_cnt + 1;
            end if;
        end if;
    end process;

    process(scan_cnt, hr_t, hr_u, min_t, min_u, sec_t, sec_u)
    begin
       -- 位选输出 (根据你的硬件，改为高电平有效)
        sel_out <= (others => '0');
        sel_out(scan_cnt) <= '1';
        
        -- 数据多路选择
        case scan_cnt is
            when 7 => current_num <= hr_t;    -- 最左侧显示小时十位
            when 6 => current_num <= hr_u;    -- 小时个位
            when 5 => current_num <= 10;      -- 分隔符 "-"
            when 4 => current_num <= min_t;   -- 分钟十位
            when 3 => current_num <= min_u;   -- 分钟个位
            when 2 => current_num <= 10;      -- 分隔符 "-"
            when 1 => current_num <= sec_t;   -- 秒钟十位
            when 0 => current_num <= sec_u;   -- 秒钟个位
        end case;
    end process;

    ---------------------------------------------------------
    -- 4. 闪烁控制逻辑
    ---------------------------------------------------------
    process(state, scan_cnt, clk_1hz)
    begin
        blink_en <= '0';
        -- 当处于调时状态，且到了闪烁的暗半周期 (clk_1hz = '1')
        if clk_1hz = '1' then
            if (state = 1 or state = 3) and (scan_cnt = 7 or scan_cnt = 6) then
                blink_en <= '1'; -- 小时位隐去
            elsif (state = 2 or state = 4) and (scan_cnt = 4 or scan_cnt = 3) then
                blink_en <= '1'; -- 分钟位隐去
            end if;
        end if;
    end process;

    ---------------------------------------------------------
    -- 5. 七段译码器 (共阳极: 0亮1灭)
    ---------------------------------------------------------
    process(current_num, blink_en, is_pm, is_12hr_fmt, scan_cnt)
        variable seg_temp : std_logic_vector(7 downto 0);
    begin
        if blink_en = '1' then
            seg_temp := x"FF"; -- 全灭 (闪烁掩码)
        else
            -- 译码: DP, G, F, E, D, C, B, A
            case current_num is
                when 0 => seg_temp := "11000000"; -- C0
                when 1 => seg_temp := "11111001"; -- F9
                when 2 => seg_temp := "10100100"; -- A4
                when 3 => seg_temp := "10110000"; -- B0
                when 4 => seg_temp := "10011001"; -- 99
                when 5 => seg_temp := "10010010"; -- 92
                when 6 => seg_temp := "10000010"; -- 82
                when 7 => seg_temp := "11111000"; -- F8
                when 8 => seg_temp := "10000000"; -- 80
                when 9 => seg_temp := "10010000"; -- 90
                when 10 => seg_temp := "10111111"; -- BF (横杠 "-")
                when others => seg_temp := "11111111"; -- FF (空白)
            end case;
        end if;
        
        -- 下午(PM)指示：在12小时制下，如果是下午，点亮最左侧(scan_cnt=7)的小数点
        if is_12hr_fmt = '1' and is_pm = '1' and scan_cnt = 7 and blink_en = '0' then
            seg_temp(7) := '0'; -- 小数点 DP 置低电平点亮
        end if;

        seg_out <= seg_temp;
    end process;

end rtl;