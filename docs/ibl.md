Image based Lighting
需要计算得是渲染方程

CookBrdf
$f(N,V,L) =k_df_d + k_sf_s  $
$k_d = 1 - k_s$
$k_s = Fresnel$

对于cook brdf，我们需要计算的是
$$\begin{aligned}
L_o\left(p, \omega_o\right) & =\int_{\Omega} f_r\left(p, \omega_i, \omega_o\right) L_i\left(p, \omega_i\right) n \cdot \omega_i \mathrm{~d} \omega_i \\
& =\int_{\Omega}\left(k_d f_d+f_s\right) L_i\left(\omega_i\right) n \cdot \omega_i \mathrm{~d} \omega_i \\
& =\int_{\Omega} k_d f_d L_i\left(\omega_i\right) n \cdot \omega_i \mathrm{~d} \omega_i+\int_{\Omega} f_s L_i\left(\omega_i\right) n \cdot \omega_i \mathrm{~d} \omega_i \\
& =L_d\left(\omega_o\right)+L_s\left(\omega_o\right)
\end{aligned}$$

实时过程中，我们生成得是有光照方向L得样本，V动态更新得，先看看前面得漫反射项有没有可能预计算出

$$\begin{aligned}
L_d\left(\omega_o\right) & =\int_{\Omega} k_d f_d L_i\left(\omega_i\right) n \cdot \omega_i \mathrm{~d} \omega_i \\
& =\frac{\rho}{\pi} \int_{\Omega} k_d L_i\left(\omega_i\right) n \cdot \omega_i \mathrm{~d} \omega_i
\end{aligned}$$

$k_d = (1-)