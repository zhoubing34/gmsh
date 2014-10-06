%%%%%%%%%%%%%%%%%%%%%%%
%% GetDP vs SmallFEM %%
%% EMDA: 4 domains   %%
%%%%%%%%%%%%%%%%%%%%%%%

clear all;
close all;

getdp = ...
    [
        6.440283576083e+01
        4.516919599531e+01
        3.558730605737e+01
        5.476260163291e+00
        2.989868943615e+00
        1.914555390383e+00
        1.227779673884e+00
        1.005783504056e+00
        8.052518088429e-01
        6.680534092345e-01
        4.824882050401e-01
        3.692633020987e-01
        3.076075847501e-01
        2.369856897912e-01
        1.847992730586e-01
        1.548859871502e-01
        1.242434316976e-01
        1.047878605321e-01
        8.091896211551e-02
        6.903649795068e-02
        3.479895146750e-02
        2.375469951973e-02
        1.342274709755e-02
        1.029747197634e-02
        2.930565910037e-03
        2.097338354494e-03
        1.434591443052e-03
        1.108375466796e-03
        9.130953046911e-04
        7.141057432226e-04
        6.082163792614e-04
        4.246769429587e-04
        3.092269758202e-04
        2.533629247668e-04
        1.769716151383e-04
        1.271607343113e-04
        6.732352326335e-05
        5.094404393616e-05
        3.423064304576e-05
        2.691906386260e-05
        1.608566709204e-05
        1.157963972185e-05
        2.530030556336e-06
        1.808667690383e-06
        9.853700670853e-07
        7.535575492828e-07
        7.972146603648e-08
        5.648852975799e-08
    ];

smallfem = ...
    [
        4.553968189413e+01
        3.193944478903e+01
        2.516402543733e+01
        3.872300697005e+00
        2.114156604889e+00
        1.353795099497e+00
        8.681713332064e-01
        7.111963361233e-01
        5.693990145956e-01
        4.723850958645e-01
        3.411706816264e-01
        2.611085849573e-01
        2.175114091212e-01
        1.675741882955e-01
        1.306728191381e-01
        1.095209318247e-01
        8.785337307126e-02
        7.409620676826e-02
        5.721834683845e-02
        4.881617585029e-02
        2.460657456085e-02
        1.679710911546e-02
        9.491315494828e-03
        7.281412263552e-03
        2.072223027702e-03
        1.483042172905e-03
        1.014409337615e-03
        7.837398086723e-04
        6.456558818170e-04
        5.049490135171e-04
        4.300739262046e-04
        3.002919461797e-04
        2.186564915283e-04
        1.791546422039e-04
        1.251378291418e-04
        8.991621753211e-05
        4.760491983289e-05
        3.602287892832e-05
        2.420471982196e-05
        1.903465260040e-05
        1.137428428060e-05
        8.188041770946e-06
        1.789001762997e-06
        1.278921188784e-06
        6.967618564383e-07
        5.328456531445e-07
        5.637158922322e-08
        3.994342244780e-08
    ];

it = [1:size(smallfem, 1)];

semilogy(it, ...
         getdp,              '-o', 'linewidth', 3, ...
         smallfem,           '-o', 'linewidth', 3, ...
         smallfem * sqrt(2), '*',  'linewidth', 3);
legend({'GetDP', 'SmallFEM', 'SmallFEM * \sqrt{2}'});
title('EMDA: 4 domains');
xlabel('Iteration');
ylabel('Residual');

print('emda.png');