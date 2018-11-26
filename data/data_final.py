# -*- coding: UTF-8 -*-
# libraries and data
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns
import glob
import os
import sys

from matplotlib import colors as mcolors
colors = dict(mcolors.BASE_COLORS, **mcolors.CSS4_COLORS)

def read_battery (main_path, teste, scenario, protocol, seeds, type):
    columns = ["Dispositivo", "Tempo", "Consumo (J)"]
    adhoc = []
    wifi = []
    for seed in seeds:
        if teste:
            print seed
        try:
            file_adhoc = open(main_path+scenario+"/"+protocol+"/"+str(seed)+"/consumption_"+type+"_adhoc.txt", 'r')
            file_wifi = open(main_path+scenario+"/"+protocol+"/"+str(seed)+"/consumption_"+type+"_wifi.txt", 'r')
        except IOError:
            if teste:
                print "ERROR"
            return
        for line in file_adhoc:
            point = [float(x) for x in line.split(',')]
            adhoc.append(["ADHOC", point[0], point[1]])

        for line in file_wifi:
            point = [float(x) for x in line.split(',')]
            wifi.append(["WIFI", point[0], point[1]])


        wifi = wifi[:len(wifi)-2]
        adhoc = adhoc[:len(adhoc)-2]

    data = []
    for a in adhoc:
        data.append (a)
    for w in wifi:
        data.append (w)

    if teste:
        print data

    df = pd.DataFrame(data=data, columns=np.array(columns))
    return df
def grafico_bateria (main_path, teste, scenario, protocol, type, df):
    plt.clf()
    ax = sns.boxplot(x="Tempo", y="Consumo (J)", hue="Dispositivo", data=df, palette="Set1", showfliers=False)
    plt.setp(ax.get_xticklabels(), rotation=45, fontsize=7)
    plt.title(u"Comparação do consumo por tempo e dispositivo")
    if type == "atual":
        plt.ylim(top=15)
    plt.savefig(main_path+scenario+'/consumo_'+type+'_'+protocol+'.svg')
    plt.savefig(main_path+scenario+'/consumo_'+type+'_'+protocol+'.png')
    plt.savefig(main_path+scenario+'/consumo_'+type+'_'+protocol+'.eps')

    plt.clf()
    ax = sns.boxplot(x="Tempo", y="Consumo (J)", hue="Dispositivo", data=df, palette="Set1")
    plt.setp(ax.get_xticklabels(), rotation=45, fontsize=7)
    plt.title(u"Comparação do consumo por tempo e dispositivo")
    if type == "atual":
        plt.ylim(top=15)
    plt.savefig(main_path+scenario+'/consumo_out_'+type+'_'+protocol+'.svg')
    plt.savefig(main_path+scenario+'/consumo_out_'+type+'_'+protocol+'.png')
    plt.savefig(main_path+scenario+'/consumo_out_'+type+'_'+protocol+'.eps')

def read_pacote (main_path, teste, scenario, protocols, seeds):
    columns = ["Protocolo", "Estado", "Quantidade de pacotes"]
    lines = range(1,len(protocols)*len(seeds)*2+1) # 2 devices
    data = []
    for key, value in protocols.iteritems():
        if teste:
            print key
        for seed in seeds:
            if teste:
                print seed
            try:
                file = open(main_path+scenario+"/"+key+"/"+str(seed)+"/final.txt", 'r')
            except IOError:
                if teste:
                    print "ERROR"
                return
            line = file.readline().strip()
            d_file = [int(x) for x in line.split(',')]
            data.append([str(key), "Tx", d_file[0]])
            data.append([str(key), "Rx", d_file[1]])

    if teste:
        print data
    dat = np.array(data)
    if teste:
        print data

    columns = np.array(columns)
    if teste:
        print columns

    df = pd.DataFrame(data=data, index=lines, columns=np.array(columns))
    return df
def grafico_pacote (main_path, teste, scenario, protocols, df):
    plt.clf()
    # Grouped boxplot
    sns.boxplot(x="Estado", y="Quantidade de pacotes", hue="Protocolo", data=df, palette="Set1", showfliers=False)
    plt.title(u"Comparação do pacote por estado e protocolo")
    plt.savefig(main_path+scenario+'/pacote.svg')
    plt.savefig(main_path+scenario+'/pacote.png')
    plt.savefig(main_path+scenario+'/pacote.eps')

    plt.clf()
    # Grouped boxplot
    sns.boxplot(x="Estado", y="Quantidade de pacotes", hue="Protocolo", data=df, palette="Set1")
    plt.title(u"Comparação do pacote por estado e protocolo")
    plt.savefig(main_path+scenario+'/pacote_out.svg')
    plt.savefig(main_path+scenario+'/pacote_out.png')
    plt.savefig(main_path+scenario+'/pacote_out.eps')

def read_consumption (main_path, teste, scenario, protocols, seeds):
    columns = ["Protocolo", "Dispositivo", "Consumo (J)"]
    lines = range(1,len(protocols)*len(seeds)*2+1) # 2 devices
    data = []
    for key, value in protocols.iteritems():
        if teste:
            print key
        for seed in seeds:
            if teste:
                print seed
            try:
                file = open(main_path+scenario+"/"+key+"/"+str(seed)+"/final.txt", 'r')
            except IOError:
                if teste:
                    print "ERROR"
                return
            line = file.readline().strip()
            line = file.readline().strip()
            d_file = [float(x) for x in line.split(',')]
            data.append([str(key), "Wifi", d_file[0]])
            data.append([str(key), "Adhoc", d_file[1]])

    if teste:
        print data
    dat = np.array(data)
    if teste:
        print data

    columns = np.array(columns)
    if teste:
        print columns

    df = pd.DataFrame(data=data, index=lines, columns=np.array(columns))
    return df
def grafico_consumption (main_path, teste, scenario, protocols, df):
    plt.clf()
    # Grouped boxplot
    sns.boxplot(x="Dispositivo", y="Consumo (J)", hue="Protocolo", data=df, palette="Set1", showfliers=False)
    plt.title(u"Comparação do consumo por interface e protocolo")
    plt.savefig(main_path+scenario+'/consumo.svg')
    plt.savefig(main_path+scenario+'/consumo.png')
    plt.savefig(main_path+scenario+'/consumo.eps')

    plt.clf()
    # Grouped boxplot
    sns.boxplot(x="Dispositivo", y="Consumo (J)", hue="Protocolo", data=df, palette="Set1")
    plt.title(u"Comparação do consumo por interface e protocolo")
    plt.savefig(main_path+scenario+'/consumo_out.svg')
    plt.savefig(main_path+scenario+'/consumo_out.png')
    plt.savefig(main_path+scenario+'/consumo_out.eps')

# ------------------------ MAIN
teste = False
#if sys.ar
# if sys.argv[1] == "False":
#     teste = False
main_path = "./output/"
try:
    file = open("seeds.txt", 'r')
except IOError:
    exit()
seeds = [int(x) for x in file.readline().strip().split(',')]
scenarios = ['teste_2', 'teste_3']
protocols = {"OLSR":"darksalmon", "AODV":"blueviolet", "DSDV":"skyblue"}
for scenario in scenarios:
    df = read_consumption (main_path, teste, scenario, protocols, seeds)
    if teste:
        print df
    grafico_consumption(main_path, teste, scenario, protocols, df)

    df = read_pacote (main_path, teste, scenario, protocols, seeds)
    if teste:
        print df
    grafico_pacote(main_path, teste, scenario, protocols, df)

    for key, value in protocols.iteritems():
        if teste:
            print key
        types = ["cresc", "atual"]
        for type in types:
            df = read_battery (main_path, teste, scenario, key, seeds, type)
            grafico_bateria (main_path, teste, scenario, key, type, df)
